//! Integration testing of the CSEc module. Tests the following:
//! - initializes the CSEc module,
//! - randomizes 128 bits of data,
//! - loads a plaintext key,
//! - encrypts a string to said key, using the randomized bits as initialization vector,
//! - decrypts the encrypted strings (ensuring the string matches before encryption and after
//! decryption),
//! - generates a MAC for a string, and
//! - verified a MAC for a string.
#![no_main]
#![no_std]

use cortex_m_rt::entry;
use s32k144;
use s32k144evb::wdog;

#[path = "../src/csec.rs"]
mod csec;
#[path = "../src/panic.rs"]
mod panic;
#[path = "../src/scg.rs"]
mod scg;
#[path = "../src/utils.rs"]
mod utils;

const MSG_LEN: usize = 16 * 10;
const PLAINKEY: [u8; 16] = [
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,
];

#[entry]
unsafe fn main() -> ! {
    let p = s32k144::Peripherals::take().unwrap();

    // Disable watchdog
    let wdog_settings = wdog::WatchdogSettings {
        enable: false,
        ..Default::default()
    };
    let _wdog = wdog::Watchdog::init(&p.WDOG, wdog_settings).unwrap();

    scg::configure_spll_clock(&p.SCG);

    let plaintext: &[u8] = "Key:0123456789abKey:0123456789abKey:0123456789abKey:0123456789abKey:0123456789abKey:0123456789abKey:0123456789abKey:0123456789abKey:0123456789abKey:0123456789ab".as_bytes();

    let mut enctext: [u8; MSG_LEN] = [0; MSG_LEN];
    let mut dectext: [u8; MSG_LEN] = [0; MSG_LEN];

    let csec = csec::CSEc::init(p.FTFC, p.CSE_PRAM);
    csec.init_rng().unwrap();
    let rnd_buf = csec.generate_rnd().unwrap();
    csec.load_plainkey(&PLAINKEY).unwrap();
    csec.encrypt_cbc(&plaintext, &rnd_buf, &mut enctext)
        .unwrap();
    csec.decrypt_cbc(&enctext, &rnd_buf, &mut dectext).unwrap();
    let cmac = csec.generate_mac(&plaintext).unwrap();

    assert!(csec.verify_mac(&plaintext, &cmac).unwrap());
    assert!(plaintext == &dectext[..]);

    // light green LED
    p.PCC.pcc_portd.modify(|_, w| w.cgc()._1());
    p.PORTD.pcr16.write(|w| w.bits(0x100));
    p.PTD.pddr.modify(|_, w| w.bits(1 << 16));

    loop {}
}
