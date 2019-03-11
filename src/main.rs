//! # Daredevil *light/small*, sensor array module
//!
//! This crate constitutes the embedded application of the sensor array module for the Daredevil
//! project, acting as the EVITA *light/small* compliant module. Once every second this application
//! 1. reads ultrasonic range sensor data from four ADC (analog-to-digital converter) channels;
//! 2. randomizes a `[u8; 16]` initialization vector in preparation for AES-CBC-128 encryption;
//! 3. encrypts the sensor data for the `PLAINKEY: [u8; 16]` constant;
//! 4. generates a MAC (message authentication code) of the initialization vector and encrypted
//!    sensor data, and
//! 5. transmits this payload over a CAN FD interface.
//!
//! This crate acts as a suitable base upon which an EVITA-compliant application can theoretically
//! be written.
//!
//! ## CAN-FD frame data
//! Every second this application sends a CAN-FD frame containing 64B of data. 40B of which
//! contains useful data. The frame contains
//! the following data:
//! - `frame[0..16]`: message authentication code;
//! - `frame[16..32]`: initialization vector for encrypted data;
//! - `frame[32..40]`: encrypted sensor data, and
//! - `frame[40..64]`: unused, always 0.
//!
//! To process a frame, the MAC (`frame[0..16]`) should be verified for `frame[16..48]` and the
//! `PLAINKEY` constant. After a message has been verified to not have been modified in transit,
//! the encrypted sensor data (`frame[32..64]`) should be encrypted using the initialization vector
//! (`frame[16..32]`) and the `PLAINKEY` constant. When the data has been decrypted, the sensor
//! data should be parsed as follows:
//! - `frame[32..34]`: sensor index 0;
//! - `frame[34..36]`: sensor index 1;
//! - `frame[36..38]`: sensor index 2, and
//! - `frame[38..40]`: sensor index 3.
//!
//! Each sensor distance is encoded in Big-Endian.
//!
//! Refer to the `csec` module for CAN-FD transmission parameters.
#![no_main]
#![no_std]

use rtfm::{app, Instant};
use s32k144::Interrupt;
use s32k144evb::wdog;
use panic_halt;

pub mod adc;
pub mod can;
pub mod csec;
pub mod scg;
pub mod utils;

const PERIOD: u32 = 10_000_000;

/// Key used for data encryption and MAC generation.
const PLAINKEY: [u8; 16] = [
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,
];

#[app(device = s32k144)]
const APP: () = {
    // Resources
    static mut ADC: adc::ADC = ();
    static mut CSEC: csec::CSEc = ();
    static mut CAN: can::CAN = ();

    #[init(schedule = [poll_sensor])]
    fn init() -> init::LateResources {
        // Disable watchdog
        let wdog_settings = wdog::WatchdogSettings {
            enable: false,
            ..Default::default()
        };
        let _wdog = wdog::Watchdog::init(&device.WDOG, wdog_settings).unwrap();

        // Configure clocks as required for CAN-FD.
        scg::configure_spll_clock(&device.SCG);

        // Initialize ADC and CAN-FD
        let adc = adc::ADC::init(&device.PCC, device.ADC0);
        let can = can::CAN::init(
            &device.PCC,
            device.CAN0,
            &device.PORTE,
            &device.PORTD,
            &device.PTD,
        );

        // Load plainkey and initialize RNG
        let csec = csec::CSEc::init(device.FTFC, device.CSE_PRAM);
        csec.init_rng().unwrap();
        csec.load_plainkey(&PLAINKEY).unwrap();

        schedule
            .poll_sensor(Instant::now() + PERIOD.cycles())
            .unwrap();

        init::LateResources {
            ADC: adc,
            CAN: can,
            CSEC: csec,
        }
    }

    #[idle]
    fn idle() -> ! {
        // Sleep
        loop {
            rtfm::pend(Interrupt::DMA0);
        }
    }

    #[task(resources = [ADC, CAN, CSEC], schedule = [poll_sensor])]
    fn poll_sensor() {
        let adc = resources.ADC;
        let can = resources.CAN;
        let csec = resources.CSEC;

        let mut payload: [u8;
            16 + // message Authentication code
            16 + // initialization vector
            adc::CHANNEL_COUNT * 2 // Two u16
        ] = [0; 32 + adc::CHANNEL_COUNT * 2];

        let mut sensor_bytes: [u8; adc::CHANNEL_COUNT * 2] = [0; adc::CHANNEL_COUNT * 2];
        u8_array_from_16_array(&adc.read(), &mut sensor_bytes);

        // Randomize our initialization vector.
        let init_vec = csec.generate_rnd().unwrap();
        payload[16..32].clone_from_slice(&init_vec);

        // Encrypt the sensor data.
        let mut encrypted: [u8; adc::CHANNEL_COUNT * 2] = [0; adc::CHANNEL_COUNT * 2];
        csec.encrypt_cbc(&sensor_bytes, &init_vec, &mut encrypted[..])
            .unwrap();
        payload[32..].clone_from_slice(&encrypted);

        // Generate a MAC (Message Authentication Code) for our payload
        let cmac = csec.generate_mac(&payload[16..]).unwrap();
        payload[..16].clone_from_slice(&cmac);

        can.transmit(&payload);

        schedule.poll_sensor(scheduled + PERIOD.cycles()).unwrap();
    }

    // Interrupt handlers used to dispatch software tasks
    extern "C" {
        fn DMA0();
    }
};

fn u8_array_from_16_array(input: &[u16], output: &mut [u8]) {
    assert!(output.len() >= input.len() * 2);

    for i in 0..input.len() {
        output[i * 2] = ((input[i] & 0xff00) >> 8) as u8;
        output[i * 2 + 1] = ((input[i] & 0xff) >> 0) as u8;
    }
}
