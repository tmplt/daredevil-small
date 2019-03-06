//! TODO: heavily document the application here.
#![no_main]
#![no_std]

use rtfm::{app, Instant};
use s32k144::Interrupt;
use s32k144evb::wdog;

pub mod adc;
pub mod can;
pub mod csec;
pub mod panic;
pub mod scg;
pub mod utils;

const PERIOD: u32 = 8_000;
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
