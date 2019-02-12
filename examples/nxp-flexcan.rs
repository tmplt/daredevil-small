//! An (almost) direct C-to-Rust rewrite of the S32K144 FlexCAN example from NXP's S32 SDK. Sets up
//! communication over CAN0 and continuously transmits messages. Sends CAN frames with a speed of
//! 50 kbit/s, SJW = 4, Tseg1 = 11, Tseg2 = 4.
//!
//! Differences from C example:
//! * CAN freeze mode is seemingly not entered correctly.
//! * no buffer/filtering clearing/initialization (see commented block); freeze mode required.
//!
//! Only used for testing/development purposes.
#![no_main]
#![no_std]

extern crate cortex_m_rt;
extern crate panic_halt;
extern crate s32k144;

use cortex_m_rt::entry;

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

    // FLEXCAN0_init
    p.PCC.pcc_flex_can0.modify(|_, w| w.cgc()._1());
    p.CAN0.mcr.modify(|_, w| w.mdis()._1());
    p.CAN0.ctrl1.modify(|_, w| w.clksrc()._0());
    p.CAN0
        .mcr
        .modify(|_, w| w.mdis()._0().frz()._1().halt()._1());
    while p.CAN0.mcr.read().frzack().is_1() {}
    p.CAN0.ctrl1.write(|w| w.bits(0xdb0006));
    // for i in 0..128 {
    //     p.CAN0.embedded_ram[i].write(|w| w.bits(0));
    // }
    // for i in 0..16 {
    //     p.CAN0.rximr[i].write(|w| w.bits(0xffffffff));
    // }
    p.CAN0.rxmgmask.write(|w| w.bits(0x1fffffff));
    p.CAN0.embedded_ram[4 * 4].modify(|_, w| w.bits(0x4000000));
    p.CAN0.embedded_ram[4 * 4 + 1].modify(|_, w| w.bits(0x14440000));
    p.CAN0.mcr.write(|w| w.bits(0x1f));
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
        p.CAN0.embedded_ram[2].write(|w| w.bits(0xa5112233));
        p.CAN0.embedded_ram[3].write(|w| w.bits(0x44556677));
        p.CAN0.embedded_ram[1].write(|w| w.bits(0x15540000));
        p.CAN0.embedded_ram[0].write(|w| w.bits(0xc400000 | (8 << 16)));
    }
}
