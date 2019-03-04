//! TODO: heavily document the application here.
#![no_main]
#![no_std]

use rtfm::{app, Instant};
use s32k144::Interrupt;
use s32k144evb::wdog;

pub mod can;
pub mod csec;
pub mod panic;
pub mod utils;

const MAGIC_SCALAR: u32 = 5000 / 0xfff;
const PERIOD: u32 = 8_000;

#[app(device = s32k144)]
const APP: () = {
    // Resources
    static mut ADC: s32k144::ADC0 = ();
    static mut CSEc: csec::CSEc = ();
    static mut CAN: can::CAN = ();

    #[init(schedule = [poll_sensor])]
    fn init() {
        // Disable watchdog
        let wdog_settings = wdog::WatchdogSettings {
            enable: false,
            ..Default::default()
        };
        let _wdog = wdog::Watchdog::init(&device.WDOG, wdog_settings).unwrap();

        // Configure clocks as required for CAN-FD.
        configure_spll_clock(&device.SCG);

        adc_init(&device.PCC, &device.ADC0);
        let can = can::CAN::init(
            &device.PCC,
            device.CAN0,
            &device.PORTE,
            &device.PORTD,
            &device.PTD,
        );

        let plainkey: [u8; 16] = [
            0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf,
            0x4f, 0x3c,
        ];

        let csec = csec::CSEc::init(device.FTFC, device.CSE_PRAM);
        csec.init_rng().unwrap();
        csec.load_plainkey(&plainkey).unwrap();

        schedule
            .poll_sensor(Instant::now() + PERIOD.cycles())
            .unwrap();

        ADC = device.ADC0;
        CAN = can;
        CSEc = csec;
    }

    #[idle]
    fn idle() -> ! {
        // Sleep
        loop {
            rtfm::pend(Interrupt::DMA0);
        }
    }

    #[task(resources = [ADC, CAN, CSEc], schedule = [poll_sensor])]
    fn poll_sensor() {
        let adc = resources.ADC;
        let can = resources.CAN;
        let csec = resources.CSEc;

        // Configure ADC0 channel 15, ADC0_SE15. Translates to pin PTC17.
        // Must be done every loop; allows us to read from the channel.
        adc.sc1a.modify(|_, w| w.adch()._00000());
        adc.sc1a.write(|w| unsafe { w.bits(15) });

        // Read sensor value.
        while adc.sc1a.read().coco().bit_is_set() == true {} // wait while clock is changed
        let sensor_values: [u8; 2] =
            split_u16_to_byte_array((MAGIC_SCALAR * adc.ra.read().bits()) as u16);

        // Randomize our initialization vector.
        let mut init_vec: [u8; 16] = [0; 16];
        csec.generate_rnd(&mut init_vec).unwrap();

        // Encrypt the sensor data.
        let mut payload: [u8; 16 + 2] = [0; 16 + 2];
        csec.encrypt_cbc(&sensor_values, &init_vec, &mut payload[16..])
            .unwrap();

        // Transmit the payload, with a prefixed initialization vector.
        payload[..16].clone_from_slice(&init_vec);
        can.transmit(&payload);

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
        #[rustfmt::skip]
        scg.soscdiv.write(|w| {
            w.soscdiv1().bits(0b001) // Divide by 1
                .soscdiv2().bits(0b001) // Divide by 1
        });
        #[rustfmt::skip]
        scg.sosccfg.write(|w| {
            w.range()._10() // set medium frequency range (4Mhz - 8Mhz)
                .hgo()._1()
                .erefs()._1()
        });
        while scg.sosccsr.read().lk().is_1() {} // Ensure SOSCCSR unlocked
        scg.sosccsr.write(|w| w.soscen()._1());
        while scg.sosccsr.read().soscvld().is_1() {}

        // SPLL_init_160Mhz
        while scg.spllcsr.read().lk().is_1() {} // Ensure SPLL unlocked
        scg.spllcsr.write(|w| w.spllen()._0()); // Disable SPLL
        #[rustfmt::skip]
        scg.splldiv.write(|w| {
            w.splldiv1().bits(0b010) // Divide by 2
                .splldiv2().bits(0b011) // Divide by 4
        });
        scg.spllcfg.write(|w| w.mult().bits(0b1_1000)); // Multiply factor 40
        while scg.spllcsr.read().lk().is_1() {}
        scg.spllcsr.write(|w| w.spllen()._1());
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

fn split_u16_to_byte_array(n: u16) -> [u8; 2] {
    let x: u8 = ((n >> 8) & 0xff) as u8;
    let y: u8 = (n & 0xff) as u8;
    [x, y]
}
