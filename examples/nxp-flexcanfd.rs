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
    p.SCG.rccr.modify(|_, w| {
        w.scs()
            .bits(6)
            .divcore()
            .bits(0b01)
            .divbus()
            .bits(0b01)
            .divslow()
            .bits(0b10)
    });
    while p.SCG.csr.read().scs().bits() != 6 {} // wait while clock is changed

    // ------------------- FLEXCAN0_init(void) -------------------

    // FLEXCAN0_init
    p.PCC.pcc_flex_can0.modify(|_, w| w.cgc()._1()); // Enable clock for CAN
    p.CAN0.mcr.modify(|_, w| w.mdis()._1()); // Disable FlexCAN module
    p.CAN0.ctrl1.modify(|_, w| w.clksrc()._1()); // Set oscillator clock to peripheral clock
    p.CAN0.mcr.modify(|_, w| w.mdis()._0()); // Reenable FlexCAN module
    while p.CAN0.mcr.read().frzack().is_0() {}

    // Configure nominal phase
    p.CAN0.cbt.write(|w| {
        w.epseg2().bits(0b01111) // Bit length of phase segment 2
            .epseg1().bits(0b01111) // Bit length of phase segment 2
            .epropseg().bits(0b101110) // Bit time length of propagation segment
            .erjw().bits(0b01111) // Extended Resync Jump Width
            .epresdiv().bits(0b00_0000_0001) // Ratio between PE clock and Sclock
            .btf()._1() // Enable bit timing format
    });

    // Configure data phase
    p.CAN0.fdcbt.write(|w| {
        w.fpseg2().bits(0b011) // Bit time length of Fast Phase Segment 2
            .fpseg1().bits(0b11) // Bit time length of Fast Phase Segment 1
            .fpropseg().bits(0b00111) // Bit time length of the propagation segment
            .frjw().bits(0b011) // Number of time quanta per resynchronization
            .fpresdiv().bits(0b00_0000_0010) // Ratio between PE clock and Sclock
    });

    p.CAN0.fdctrl.write(|w| {
        w.tdcoff().bits(0b11111) // Transceiver Delay Compensation Offset
            .tdcen()._1() // Enable TDC
            .mdbsr0()._11() // 64 bytes per message buffer
            .fdrate()._1() // Enable Bit Rate Switch
    });

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
        p.CAN0.embedded_ram[(0 * MSG_BUF_SIZE) + 2].write(|w| w.bits(0xa5112233));
        p.CAN0.embedded_ram[(0 * MSG_BUF_SIZE) + 3].write(|w| w.bits(0x44556677));
        p.CAN0.embedded_ram[(0 * MSG_BUF_SIZE) + 1].write(|w| w.bits(0x15540000));
        p.CAN0.embedded_ram[(0 * MSG_BUF_SIZE) + 0].write(|w| w.bits(0xcc4f0000 | (8 << 16)));
    }
}
