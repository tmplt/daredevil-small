//! # Daredevil *light/small*, sensor array module
//!
//! This crate constitutes the embedded application of the sensor array module for the Daredevil
//! project, acting as the EVITA *light/small* compliant module. Once every 1/8 second this application:
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
//! Every second this application sends a CAN-FD frame containing 64B of data. 48B of which
//! contains useful data. The frame contains
//! the following data:
//! - `frame[0..16]`: message authentication code (MAC);
//! - `frame[16..32]`: initialization vector (IV) for AES-CBC-128 encrypted data;
//! - `frame[32..48]`: AES encrypted sensor data, and
//! - `frame[48..64]`: unused, always 0.
//!
//! To process a frame, the MAC (`frame[0..16]`) should be verified for `frame[16..48]` and the
//! `PLAINKEY` constant. After a message has been verified to not have been modified in transit,
//! the encrypted sensor data (`frame[32..48]`) should be decrypted using the initialization vector
//! (`frame[16..32]`) and the `PLAINKEY` constant. When the data has been decrypted into a `data: [u8; 16]`,
//! data should be parsed as follows:
//! - `data[0..2]`: sensor index 0;
//! - `data[2..4]`: sensor index 1;
//! - `data[4..6]`: sensor index 2;
//! - `data[6..8]`: sensor index 3, and
//! - `data[8..16]`: unused, always 0.
//!
//! Each sensor distance is encoded in Big-Endian.
//!
//! Refer to the `csec` module for CAN-FD transmission parameters.
//!
//! ## Stack usage
//! Rust promises a lot of safety when writing a program, but those promises are invalidated if the program overflows the stack.
//! We should ensure that the stack our program uses does not exceed available memory on-board.
//! The stack usage of `daredevil-small` can be analysed via [`cargo call-stack`](https://github.com/japaric/cargo-call-stack)
//! and `cargo size`. Install the utilities via:
//! ```
//! rustup component add llvm-tools-preview
//! cargo +stable install call-stack
//! ```
//!
//! Before continuing, enable the `inline-asm` feature of the `cortex-m` dependency by altering
//! the `Cargo.toml` to contain:
//! ```
//! [dependencies.cortex-m]
//! version = "0.5.0"
//! features = ["inline-asm"]
//! ```
//! This feature will give more information to `call-stack`, allowing for a more accurate stack
//! analysis. Now, run `cargo +nightly call-stack --bin daredevil-small > cg.dot`. `cg.dot` now
//! contains a call-graph description of the program in the [DOT language](https://en.wikipedia.org/wiki/DOT_(graph_description_language)).
//! An example output (from commit `963ab26`) is the following:
//! ```
//! digraph {
//!     node [fontname=monospace shape=box]
//!     0 [label="DMA0\nmax = 240\nlocal = 208"]
//!     1 [label="main\nmax = 80\nlocal = 48"]
//!     2 [label="SysTick\nmax = 40\nlocal = 40"]
//!     3 [label="daredevil_small::csec::CSEc::write_command_bytes\nmax = 32\nlocal = 32"]
//!     4 [label="daredevil_small::__rtfm_internal_15::{{closure}}\nmax = 16\nlocal = 16"]
//!     5 [label="daredevil_small::csec::CSEc::read_command_bytes\nmax = 8\nlocal = 8"]
//!     6 [label="core::result::unwrap_failed::hbfd77e16ddb866fd\nmax = 0\nlocal = 0"]
//!     7 [label="core::result::unwrap_failed::hc349ba809e82420c\nmax = 0\nlocal = 0"]
//!     8 [label="core::result::unwrap_failed::hcece78b842d1e303\nmax = 0\nlocal = 0"]
//!     9 [label="daredevil_small::csec::CSEc::write_command_header\nmax = 0\nlocal = 0"]
//!     10 [label="Reset\nmax = 80\nlocal = 0"]
//!     11 [label="ResetTrampoline\nmax = 80\nlocal = 0"]
//!     12 [label="ADC0\nmax = 0\nlocal = 0"]
//!     13 [label="DefaultPreInit\nmax = 0\nlocal = 0"]
//!     14 [label="rust_begin_unwind\nmax = 0\nlocal = 0"]
//!     15 [label="core::panicking::panic_bounds_check\nmax = 0\nlocal = 0"]
//!     16 [label="core::panicking::panic_fmt\nmax = 0\nlocal = 0"]
//!     17 [label="core::panicking::panic\nmax = 0\nlocal = 0"]
//!     18 [label="HardFault\nmax = 0\nlocal = 0"]
//!     19 [label="__aeabi_memset4\nmax = 16\nlocal = 8"]
//!     20 [label="HardFaultTrampoline\nmax >= 0\nlocal = ?"]
//!     21 [label="__aeabi_memset\nmax = 8\nlocal = 8"]
//!     22 [label="__aeabi_memclr4\nmax = 16\nlocal = 0"]
//!     7 -> 16
//!     17 -> 16
//!     3 -> 17
//!     3 -> 15
//!     16 -> 14
//!     5 -> 17
//!     8 -> 16
//!     0 -> 16
//!     0 -> 9
//!     0 -> 7
//!     0 -> 5
//!     0 -> 3
//!     0 -> 15
//!     0 -> 8
//!     0 -> 4
//!     15 -> 16
//!     10 -> 13
//!     10 -> 11
//!     9 -> 16
//!     1 -> 16
//!     1 -> 6
//!     1 -> 9
//!     1 -> 7
//!     1 -> 3
//!     1 -> 8
//!     1 -> 4
//!     2 -> 16
//!     11 -> 1
//!     6 -> 16
//!     0 -> 22
//!     19 -> 21
//!     20 -> 18
//!     22 -> 19
//! }
//! ```
//!
//! From this file we are interested in the `max` values of `DMA0` (interrupt used for the sensor polling task),
//! `main` (the initialization function), and `HardFaultTrampoline` (exception handler entry in case of a `HardFault`);
//! these nodes denote the function entry points of `daredevil-small` which may preempt eachother.
//! From the above example, these values are 240, 80, and 0, respectively. Summing these values, we obtain the maximum stack usage of
//! `daredevil-small` during run-time. However, some additional stack is used when interrupts are handled. The exact value is documented
//! in the reference manual of the board's processor family, but here we'll use a lump sum of 1K additional stack usage for simplicity.
//!
//! The `daredevil-small` binary itself will also store some data in the board's available memory. We find the exact number of bytes via:
//! ```
//! $ cargo size --bin daredevil-small --release
//!    text    data     bss     dec     hex filename
//!   13128    3020      92   16240    3f70 daredevil-small
//! ```
//! From this output the `data` and `bss` sections will be stored in memory. The final amount of bytes that will be stored on the stack
//! is then `240 + 80 + 0 + 1000 + 3020 + 92 = 4432`. From `../memory.x` we can read that we have 16K of available memory.
//! And since `4432 <= 16000` we do not overflow the available stack and thus we have ensured the promises of programming in Rust.
#![no_main]
#![no_std]

use panic_halt;
use rtfm::{app, Instant};
use s32k144::Interrupt;
use s32k144evb::wdog;

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
            16 // encrypted sensor data
        ] = [0; 48];

        let mut sensor_bytes = [0u8; 16];
        u8_array_from_16_array(&adc.read(), &mut sensor_bytes);

        // Randomize our initialization vector.
        let init_vec = csec.generate_rnd().unwrap();
        payload[16..32].clone_from_slice(&init_vec);

        // Encrypt the sensor data.
        let mut encrypted = [0u8; 16];
        csec.encrypt_cbc(&sensor_bytes, &init_vec, &mut encrypted[..])
            .unwrap();
        payload[32..48].clone_from_slice(&encrypted);

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
