//! An example that reads a sensor on pin PTC17 and dumps its value over CAN-FD.
#![no_main]
#![no_std]

use cortex_m_rt::entry;
use s32k144;
use s32k144evb::wdog;

#[path = "../src/panic.rs"]
mod panic;

const MAGIC_SCALAR: u32 = 5000 / 0xfff;
const MSG_BUF_SIZE: usize = 18;

#[entry]
fn main() -> ! {
    let p = s32k144::Peripherals::take().unwrap();

    // Disable watchdog
    let wdog_settings = wdog::WatchdogSettings {
        enable: false,
        ..Default::default()
    };
    let _wdog = wdog::Watchdog::init(&p.WDOG, wdog_settings).unwrap();

    // Required for CAN-FD
    unsafe {
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
    }

    // ADC_init
    unsafe {
        p.PCC.pcc_adc0.modify(|_, w| w.cgc()._0()); //Disable clock
        p.PCC.pcc_adc0.modify(|_, w| w.pcs()._001()); // PCS=1
        p.PCC.pcc_adc0.modify(|_, w| w.cgc()._1()); // Enable Clock
        p.ADC0.sc1a.write(|w| w.bits(0x1F)); // ADCH=1F
        p.ADC0.cfg1.write(|w| w.bits(0x4)); // ADICLK=0
        p.ADC0.cfg2.write(|w| w.bits(0xC)); // SMPLTS=12 (default);
        p.ADC0.sc2.write(|w| w.bits(0x0)); // ADTRG=0
        p.ADC0.sc3.write(|w| w.bits(0x0)); // CAL=0
    }

    // FLEXCAN0_init
    unsafe {
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
    }

    // Initialize CAN ports
    unsafe {
        p.PCC.pcc_porte.modify(|_, w| w.cgc()._1());
        p.PORTE.pcr4.modify(|_, w| w.mux()._101());
        p.PORTE.pcr5.modify(|_, w| w.mux()._101());
        p.PCC.pcc_portd.modify(|_, w| w.cgc()._1());
        p.PORTD.pcr16.write(|w| w.bits(0x100));
        p.PTD.pddr.modify(|_, w| w.bits(1 << 16));
    }

    loop {
        // Configure ADC0 channel 15, ADC0_SE15. Translates to pin PTC17.
        // Must be done every loop; allows us to read from the channel.
        p.ADC0.sc1a.modify(|_, w| w.adch()._00000());
        p.ADC0.sc1a.write(|w| unsafe { w.bits(15) });

        // Read sensor value.
        while p.ADC0.sc1a.read().coco().bit_is_set() == true {} // wait while clock is changed
        let sensor_value: u32 = MAGIC_SCALAR * p.ADC0.ra.read().bits();
        transmit_over_can(&p.CAN0, sensor_value);
    }
}

fn transmit_over_can(can: &s32k144::CAN0, payload: u32) {
    // FLEXCAN0_transmit_msg
    unsafe {
        can.iflag1.write(|w| w.bits(0x1));
        can.embedded_ram[(0 * MSG_BUF_SIZE) + 2].write(|w| w.bits(0xa5112233));
        can.embedded_ram[(0 * MSG_BUF_SIZE) + 3].write(|w| w.bits(payload));
        can.embedded_ram[(0 * MSG_BUF_SIZE) + 1].write(|w| w.bits(0x15540000));
        can.embedded_ram[(0 * MSG_BUF_SIZE) + 0].write(|w| w.bits(0xcc4f0000 | (8 << 16)));
    }
}
