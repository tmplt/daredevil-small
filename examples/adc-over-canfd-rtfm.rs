//! An example that reads a sensor on pin PTC17 and dumps its value over CAN-FD, wrapped in RTFM.
#![no_main]
#![no_std]

use rtfm::{app, Instant};
use s32k144::Interrupt;
use s32k144evb::wdog;

#[path = "../src/panic.rs"]
mod panic;

const MAGIC_SCALAR: u32 = 5000 / 0xfff;
const MSG_BUF_SIZE: usize = 18;
const PERIOD: u32 = 16_000_000;

#[app(device = s32k144)]
const APP: () = {
    // Resources
    static mut ADC: s32k144::ADC0 = ();
    static mut CAN: s32k144::CAN0 = ();

    #[init(schedule = [poll_sensor])]
    fn init() {
        // Disable watchdog
        let wdog_settings = wdog::WatchdogSettings {
            enable: false,
            ..Default::default()
        };
        let _wdog = wdog::Watchdog::init(&device.WDOG, wdog_settings).unwrap();

        // Settings required for CAN-FD
        configure_spll_clock(&device.SCG);

        adc_init(&device.PCC, &device.ADC0);
        flexcan0_init(
            &device.PCC,
            &device.CAN0,
            &device.PORTE,
            &device.PORTD,
            &device.PTD,
        );

        schedule
            .poll_sensor(Instant::now() + PERIOD.cycles())
            .unwrap();

        ADC = device.ADC0;
        CAN = device.CAN0;
    }

    #[idle]
    fn idle() -> ! {
        // Sleep
        loop {
            rtfm::pend(Interrupt::DMA0);
        }
    }

    #[task(resources = [ADC, CAN], schedule = [poll_sensor])]
    fn poll_sensor() {
        let adc = resources.ADC;
        let can = resources.CAN;

        // Configure ADC0 channel 15, ADC0_SE15. Translates to pin PTC17.
        // Must be done every loop; allows us to read from the channel.
        adc.sc1a.modify(|_, w| w.adch()._00000());
        adc.sc1a.write(|w| unsafe { w.bits(15) });

        // Read sensor value.
        while adc.sc1a.read().coco().bit_is_set() == true {} // wait while clock is changed
        let sensor_value: u32 = MAGIC_SCALAR * adc.ra.read().bits();
        transmit_over_can(&can, sensor_value);

        schedule.poll_sensor(scheduled + PERIOD.cycles()).unwrap();
    }

    // Interrupt handlers used to dispatch software tasks
    extern "C" {
        fn DMA0();
    }
};

fn configure_spll_clock(scg: &s32k144::SCG) {
    unsafe {
        // SOSC_init_8Mhz
        scg.soscdiv.write(|w| w.bits(0x101));
        scg.sosccfg.write(|w| w.bits(0x24));
        while scg.sosccsr.read().lk().is_1() {} // Ensure SOSCCSR unlocked
        scg.sosccsr.write(|w| w.bits(0x1));
        while scg.sosccsr.read().soscvld().is_1() {}

        // SPLL_init_160Mhz
        while scg.spllcsr.read().lk().is_1() {}
        scg.spllcsr.write(|w| w.bits(0x0));
        scg.splldiv.write(|w| w.bits(0x302));
        scg.spllcfg.write(|w| w.bits(0x180000));
        while scg.spllcsr.read().lk().is_1() {}
        scg.spllcsr.write(|w| w.bits(0x1));
        while scg.spllcsr.read().spllvld().is_1() {}

        // NormalRUNmode_80Mhz
        #[rustfmt::skip]
        scg.rccr.modify(|_, w| {
            w.scs().bits(6)
                .divcore().bits(0b01)
                .divbus().bits(0b01)
                .divslow().bits(0b10)
        });
        while scg.csr.read().scs().bits() != 6 {} // wait while clock is changed
    }
}

fn adc_init(pcc: &s32k144::PCC, adc: &s32k144::ADC0) {
    unsafe {
        pcc.pcc_adc0.modify(|_, w| w.cgc()._0()); //Disable clock
        pcc.pcc_adc0.modify(|_, w| w.pcs()._001()); // PCS=1
        pcc.pcc_adc0.modify(|_, w| w.cgc()._1()); // Enable Clock
        adc.sc1a.write(|w| w.bits(0x1F)); // ADCH=1F
        adc.cfg1.write(|w| w.bits(0x4)); // ADICLK=0
        adc.cfg2.write(|w| w.bits(0xC)); // SMPLTS=12 (default);
        adc.sc2.write(|w| w.bits(0x0)); // ADTRG=0
        adc.sc3.write(|w| w.bits(0x0)); // CAL=0
    }
}

fn flexcan0_init(
    pcc: &s32k144::PCC,
    can: &s32k144::CAN0,
    porte: &s32k144::PORTE,
    portd: &s32k144::PORTD,
    ptd: &s32k144::PTD,
) {
    unsafe {
        pcc.pcc_flex_can0.modify(|_, w| w.cgc()._1());
        can.mcr.modify(|_, w| w.mdis()._1());
        can.ctrl1.modify(|_, w| w.clksrc()._1());
        can.mcr.modify(|_, w| w.mdis()._0());
        while can.mcr.read().frzack().is_0() {}

        // Configure nominal phase
        can.cbt.write(|w| w.bits(0x802FB9EF));

        // Configure data phase
        can.fdcbt.write(|w| w.bits(0x00131CE3));
        can.fdctrl.write(|w| w.bits(0x80039F00));

        // Clear 128 words RAM in module
        for i in 0..128 {
            can.embedded_ram[i].write(|w| w.bits(0));
        }
        // init CAN0 16 msg buf filters
        for i in 0..16 {
            can.rximr[i].write(|w| w.bits(0xFFFFFFFF));
        }

        // Global acceptance mask: check all ID bits
        can.rxmgmask.write(|w| w.bits(0x1FFFFFFF));

        // Message Buffer 4 - receive setup
        can.embedded_ram[(4 * MSG_BUF_SIZE) + 0].modify(|_, w| w.bits(0xC4000000));
        // Msg buf 4, word 1: standard ID = 0x511
        can.embedded_ram[(4 * MSG_BUF_SIZE) + 1].modify(|_, w| w.bits(0x14440000));

        // Enable CRC fix for ISO CAN FD
        can.ctrl2.modify(|_, w| w.isocanfden()._1());

        // Negate FlexCAN 1 halt state & enable CAN FD for 32 MBs
        can.mcr.write(|w| w.bits(0x0000081F));

        // Wait for FRZACK to clear and module ready
        while can.mcr.read().frzack().is_1() {}
        while !can.mcr.read().notrdy().is_0() {}
    }

    // Initialize CAN ports, and light green LED
    unsafe {
        pcc.pcc_porte.modify(|_, w| w.cgc()._1());
        porte.pcr4.modify(|_, w| w.mux()._101());
        porte.pcr5.modify(|_, w| w.mux()._101());
        pcc.pcc_portd.modify(|_, w| w.cgc()._1());
        portd.pcr16.write(|w| w.bits(0x100));
        ptd.pddr.modify(|_, w| w.bits(1 << 16));
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
