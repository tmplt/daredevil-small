//! A C-to-Rust rewrite (sans API) of the S32K144 security_pal example from NXP's S32 SDK.
//! Initializes the CSEC module, generates a random number, loads a plain-text key, encrypts a
//! byte-array via AES CBC, decrypts it, and compares the strings.
//!
//! Only used for testing/development purposes.
#![no_main]
#![no_std]

use cortex_m_rt::entry;
use s32k144;

#[path = "../src/csec.rs"]
mod csec;
#[path = "../src/panic.rs"]
mod panic;
#[path = "../src/utils.rs"]
mod utils;

#[entry]
unsafe fn main() -> ! {
    let p = s32k144::Peripherals::take().unwrap();

    // Disable watchdog
    p.WDOG.cnt.write(|w| w.bits(0xd928c520)); // unlock, magical number
    p.WDOG.toval.write(|w| w.bits(0xffff)); // maximum timeout value
    #[rustfmt::skip]
    p.WDOG.cs.write(|w| {
        w.en()._0() // Disable Watchdog
            .clk().bits(0b01) // Set Watchdog clock to LPO
            .cmd32en()._1() // Enable 32-bit refresh/unlock support
    });

    // SOSC_init_8Mhz
    #[rustfmt::skip]
    p.SCG.soscdiv.write(|w| {
        w.soscdiv1().bits(0b001) // Divide by 1
            .soscdiv2().bits(0b001) // Divide by 1
    });
    #[rustfmt::skip]
    p.SCG.sosccfg.write(|w| {
        w.bits(0x24) // XXX: Bit 6 is unused in this field.
        //w.hgo()._1() // Set crystal oscillator for high gain
    });
    while p.SCG.sosccsr.read().lk().is_1() {} // Ensure SOSCCSR unlocked
    p.SCG.sosccsr.write(|w| w.soscen()._1());
    while p.SCG.sosccsr.read().soscvld().is_1() {} // Validate enabling of SysOSC

    // SPLL_init_160Mhz
    while p.SCG.spllcsr.read().lk().is_1() {} // Ensure the System Phase-locked loop is unlocked
    p.SCG.spllcsr.write(|w| w.spllen()._0()); // Disable SPLL
    #[rustfmt::skip]
    p.SCG.splldiv.write(|w| {
        w.splldiv1().bits(0b010) // Divide by 2
            .splldiv2().bits(0b011) // Divide by 4
    });
    p.SCG.spllcfg.write(|w| w.mult().bits(0b11000)); // Muliply factor 40
    while p.SCG.spllcsr.read().lk().is_1() {} // Ensure Control Status Register is unlocked
    p.SCG.spllcsr.write(|w| w.spllen()._1()); // Enable CSR
    while p.SCG.spllcsr.read().spllvld().is_1() {}

    // NormalRUNmode_80Mhz
    #[rustfmt::skip]
    p.SCG.rccr.modify(|_, w| {
        w.scs().bits(6) // System PLL
         .divcore().bits(0b01) // Divide Core Clock by 2
         .divbus().bits(0b01) // Divide Bus Clock by 2
         .divslow().bits(0b10) // Divide Slow Clock by 3
    });
    while p.SCG.csr.read().scs().bits() != 6 {} // Ensure clock is configured to System PLL

    const MSG_LEN: usize = 16;
    const RND_BUF_LEN: usize = 16;
    let mut rnd_buf: [u8; RND_BUF_LEN] = [0; RND_BUF_LEN];
    const PLAINKEY: [u8; MSG_LEN] = [
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f,
        0x3c,
    ];
    let plaintext: &[u8] = "Key:0123456789abKey:0123456789abKey:0123456789abKey:0123456789abKey:0123456789abKey:0123456789abKey:0123456789abKey:0123456789abKey:0123456789abKey:0123456789ab6666666".as_bytes();
    let mut enctext: [u8; MSG_LEN * 10 + 7] = [0; MSG_LEN * 10 + 7];
    let mut dectext: [u8; MSG_LEN * 10 + 7] = [0; MSG_LEN * 10 + 7];

    assert_eq!(plaintext.len(), MSG_LEN * 10 + 7);

    let csec = csec::CSEc::init(p.FTFC, p.CSE_PRAM);
    csec.init_rng().unwrap();
    csec.generate_rnd(&mut rnd_buf).unwrap();
    csec.load_plainkey(&PLAINKEY).unwrap();
    csec.encrypt_cbc(&plaintext, &rnd_buf, &mut enctext)
        .unwrap();
    csec.decrypt_cbc(&enctext, &rnd_buf, &mut dectext).unwrap();
    assert!(plaintext == &dectext[..]);

    // light green LED
    p.PCC.pcc_portd.modify(|_, w| w.cgc()._1());
    p.PORTD.pcr16.write(|w| w.bits(0x100));
    p.PTD.pddr.modify(|_, w| w.bits(1 << 16));

    loop {}
}
