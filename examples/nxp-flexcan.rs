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
    p.WDOG.cnt.write(|w| w.bits(0xd928c520)); // unlock, magical number
    p.WDOG.toval.write(|w| w.bits(0xffff)); // maximum timeout value
    p.WDOG.cs.write(|w| {
        w.en()._0() // Disable Watchdog
            .clk().bits(0b01) // Set Watchdog clock to LPO
            .cmd32en()._1() // Enable 32-bit refresh/unlock support
    });

    // SOSC_init_8Mhz
    p.SCG.soscdiv.write(|w| {
        w.soscdiv1().bits(0b001) // Divide by 1
            .soscdiv2().bits(0b001) // Divide by 1
    });
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

    // FLEXCAN0_init
    p.PCC.pcc_flex_can0.write(|w| w.cgc()._1()); // Enable clock for CAN
    p.CAN0.mcr.write(|w| w.mdis()._1()); // Disable FlexCAN module
    p.CAN0.ctrl1.write(|w| w.clksrc()._0()); // Set oscillator clock to be CAN engine clock
    p.CAN0.mcr.write(|w| {
        w.mdis()._0() // Reenable FlexCAN, this enables freeze mode and halt
    });

    let _ = 0;

    // TODO: Lookup why this does not work in the SVD code
    // .frzack().is_1()
    while p.CAN0.mcr.read().bits() != 0x5980_000f {}

    p.CAN0.ctrl1.write(|w| { // Set Bus Speed to 500kbit/s
        w.propseg().bits(0b110) // propagation segment time = (PROPSEG+1) * one Sclock period
            .pseg2().bits(0b011) // length of phase segment 2
            .pseg1().bits(0b011) // length of phase segment 1
            .rjw().bits(0b11)   // Resync Jump Width
    });

    // Enable read
    // for i in 0..128 {
    //     p.CAN0.embedded_ram[i].write(|w| w.bits(0));
    // }
    // for i in 0..16 {
    //     p.CAN0.rximr[i].write(|w| w.bits(0xffffffff));
    // }

    p.CAN0.rxmgmask.write(|w| w.bits(0x1fffffff));
    p.CAN0.embedded_ram[4 * 4].write(|w| w.bits(0x0400_0000));
    p.CAN0.embedded_ram[4 * 4 + 1].write(|w| w.bits(0x1444_0000));
    p.CAN0.mcr.write(|w| {
        w.bits(0x0000_0000) // Just set everything to zero, takes us out of supervisor mode
            .maxmb().bits(0x1f) // Sets the number of the last message buffer (4*5 = 20MB)
    });

    let _ = 0;

    // TODO: Yet again, does not work, guessing the SVD file is wrong
    //while p.CAN0.mcr.read().frzack().is_1() {}
    //while !p.CAN0.mcr.read().notrdy().is_0() {}
    while p.CAN0.mcr.read().bits() != 0x0000_001f{}

    // PORT_init
    p.PCC.pcc_porte.modify(|_, w| w.cgc()._1()); // Enable PortE
    p.PORTE.pcr4.modify(|_, w| w.mux()._101()); // Maybe enable CAN RX
    p.PORTE.pcr5.modify(|_, w| w.mux()._101()); // Maybe enable CAN TX
    p.PCC.pcc_portd.modify(|_, w| w.cgc()._1()); // Enable PortD
    p.PORTD.pcr16.write(|w| w.bits(0x100)); // Maybe enable Green LED
    p.PTD.pddr.modify(|_, w| w.bits(1 << 16)); // Configure pin 16 to be output

    // just transmit messages in a loop down here
    loop {
        // FLEXCAN0_transmit_msg
        p.CAN0.iflag1.write(|w| w.bits(0x1));
        p.CAN0.embedded_ram[2].write(|w| w.bits(0xa511_2233));
        p.CAN0.embedded_ram[3].write(|w| w.bits(0x4455_6677));
        p.CAN0.embedded_ram[1].write(|w| w.bits(0x1554_0000));
        p.CAN0.embedded_ram[0].write(|w| w.bits(0x0c40_0000 | (8 << 16)));
    }
}
