//! An example that reads a sensor on pin PTC17 and dumps its value over serial.
#![no_main]
#![no_std]

use cortex_m_rt::entry;
use embedded_types::io::Write;
use s32k144;
use s32k144evb::{pcc, pcc::Pcc, spc, wdog};

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

    let pc_config = spc::Config {
        system_oscillator: spc::SystemOscillatorInput::Crystal(8_000_000),
        soscdiv2: spc::SystemOscillatorOutput::Div1,
        ..Default::default()
    };
    let spc = spc::Spc::init(&p.SCG, &p.SMC, &p.PMC, pc_config).unwrap();

    let pcc = Pcc::init(&p.PCC);
    let _pcc_lpuart1 = pcc.enable_lpuart1(pcc::ClockSource::Soscdiv2).unwrap();
    let _pcc_portc = pcc.enable_portc().unwrap();

    let portc = p.PORTC;
    portc.pcr6.modify(|_, w| w.mux()._010());
    portc.pcr7.modify(|_, w| w.mux()._010());

    let mut console = s32k144evb::console::LpuartConsole::init(&p.LPUART1, &spc);

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

    loop {
        // Configure ADC0 channel 15, ADC0_SE15. Translates to pin PTC17.
        // Must be done every loop; allows us to read from the channel.
        p.ADC0.sc1a.modify(|_, w| w.adch()._00000());
        p.ADC0.sc1a.write(|w| unsafe { w.bits(15) });

        // Read sensor value.
        while p.ADC0.sc1a.read().coco().bit_is_set() == true {} // wait while clock is changed
        let sensor_value: u32 = MAGIC_SCALAR * p.ADC0.ra.read().bits();
        writeln!(console, "payload: {}", sensor_value).unwrap();
    }
}
