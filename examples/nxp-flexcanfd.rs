//! A direct C-to-Rust rewrite of the S32K144 FlexCAN FD example from NXP's S32 SDK. Sets up
//! communication over CAN0 and continuously transmits messages. Sends CAN FD frames with the
//! following settings:
//!
//!     Arbitration/nominal phase: 500 kbit/s, 80% sampling point, SJW = 16.
//!     Data phase: 2000 kbit/s, 80% sampling point, SJW = 4.
//!
//! Only used for testing/development purposes.
#![no_main]
#![no_std]

extern crate cortex_m_rt;
extern crate panic_halt;
extern crate s32k144;

use cortex_m_rt::entry;

const MSG_BUF_SIZE: usize = 18;

#[entry]
unsafe fn main() -> ! {
    let p = s32k144::Peripherals::take().unwrap();

    // Disable watchdog
    p.WDOG.cnt.write(|w| w.bits(0xd928c520)); // unlock
    p.WDOG.toval.write(|w| w.bits(0xffff)); // maximum timeout value
    p.WDOG.cs.write(|w| w.bits(0x2100)); // disable watchdod

    // SOSC_init_8Mhz
    p.SCG.soscdiv.write(|w| w.bits(0x101));
    p.SCG.sosccfg.write(|w| w.bits(0x24));
    while p.SCG.sosccsr.read().lk().is_1() {} // Ensure SOSCCSR unlocked
    p.SCG.sosccsr.write(|w| w.bits(0x1));
    while p.SCG.sosccsr.read().soscvld().is_1() {}

    // SPLL_init_160Mhz
    while p.SCG.spllcsr.read().lk().is_1() {}
    p.SCG.spllcsr.write(|w| w.bits(0x0));
    p.SCG.splldiv.write(|w| w.bits(0x302));
    p.SCG.spllcfg.write(|w| w.bits(0x180000));
    while p.SCG.spllcsr.read().lk().is_1() {}
    p.SCG.spllcsr.write(|w| w.bits(0x1));
    while p.SCG.spllcsr.read().spllvld().is_1() {}

    // NormalRUNmode_80Mhz
    #[rustfmt::skip]
    p.SCG.rccr.modify(|_, w| {
        w.scs().bits(6)
            .divcore().bits(0b01)
            .divbus().bits(0b01)
            .divslow().bits(0b10)
    });
    while p.SCG.csr.read().scs().bits() != 6 {} // wait while clock is changed

    // ------------------- FLEXCAN0_init(void) -------------------

    // FLEXCAN0_init
    p.PCC.pcc_flex_can0.modify(|_, w| w.cgc()._1());
    p.CAN0.mcr.modify(|_, w| w.mdis()._1());
    p.CAN0.ctrl1.modify(|_, w| w.clksrc()._1());
    p.CAN0.mcr.modify(|_, w| w.mdis()._0());
    while p.CAN0.mcr.read().frzack().is_0() {}

    // Configure nominal phase
    p.CAN0.cbt.write(|w| w.bits(0x802FB9EF));

    // Configure data phase
    p.CAN0.fdcbt.write(|w| w.bits(0x00131CE3));
    p.CAN0.fdctrl.write(|w| w.bits(0x80039F00));

    let _ = 0;

    // Clear 128 words RAM in module
    for i in 0..128 {
        p.CAN0.embedded_ram[i].write(|w| w.bits(0));
    }
    // init CAN0 16 msg buf filters
    for i in 0..16 {
        p.CAN0.rximr[i].write(|w| w.bits(0xFFFFFFFF));
    }

    // Global acceptance mask: check all ID bits
    p.CAN0.rxmgmask.write(|w| w.bits(0x1FFFFFFF));

    // Message Buffer 4 - receive setup
    p.CAN0.embedded_ram[(4 * MSG_BUF_SIZE) + 0].modify(|_, w| w.bits(0xC4000000));
    // Msg buf 4, word 1: standard ID = 0x511
    p.CAN0.embedded_ram[(4 * MSG_BUF_SIZE) + 1].modify(|_, w| w.bits(0x14440000));

    // Enable CRC fix for ISO CAN FD
    p.CAN0.ctrl2.modify(|_, w| w.isocanfden()._1());

    // Negate FlexCAN 1 halt state & enable CAN FD for 32 MBs
    p.CAN0.mcr.write(|w| w.bits(0x0000081F));

    // Wait for FRZACK to clear and module ready
    while p.CAN0.mcr.read().frzack().is_1() {}
    while !p.CAN0.mcr.read().notrdy().is_0() {}

    // PORT_init
    p.PCC.pcc_porte.modify(|_, w| w.cgc()._1());
    p.PORTE.pcr4.modify(|_, w| w.mux()._101());
    p.PORTE.pcr5.modify(|_, w| w.mux()._101());
    p.PCC.pcc_portd.modify(|_, w| w.cgc()._1());
    p.PORTD.pcr16.write(|w| w.bits(0x100));
    p.PTD.pddr.modify(|_, w| w.bits(1 << 16));

    // just transmit messages in a loop down here
    loop {
        // FLEXCAN0_transmit_msg
        p.CAN0.iflag1.write(|w| w.bits(0x1));
        p.CAN0.embedded_ram[(0 * MSG_BUF_SIZE) + 2].write(|w| w.bits(0xa5112233));
        p.CAN0.embedded_ram[(0 * MSG_BUF_SIZE) + 3].write(|w| w.bits(0x44556677));
        p.CAN0.embedded_ram[(0 * MSG_BUF_SIZE) + 1].write(|w| w.bits(0x15540000));
        p.CAN0.embedded_ram[(0 * MSG_BUF_SIZE) + 0].write(|w| w.bits(0xcc4f0000 | (8 << 16)));
    }
}
