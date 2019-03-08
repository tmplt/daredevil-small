//! This module defines a panic handler and a HardFault handler. Upon a panic, peripherals are set
//! up so that every second, a yellow LED is toggled, and the `&PanicInfo` struct is written to
//! LPUART1. This conveniently doesn't require the developer to always be attached to the console
//! upon a panic. Upon a HardFault, a red LED is lit; a GDB client should be attached at this
//! stage.
//!
//! TODO: find some way to write HardFault debug info over serial. FreeRTOS apparently manages
//! this, and so could we.
//!
//! Note: this panic handler panics seemingly if the clock has been configured in a manner that is
//! unsupported by the board support crate.
//!
//! ---
//!
//! With the panic handler being `#[inline(never)]` the symbol `rust_begin_unwind` will be
//! available to place a breakpoint on to halt when a panic is happening.

use core::{panic::PanicInfo, ptr};
use cortex_m;
use cortex_m_rt::exception;
use embedded_types::io::Write;
use s32k144;
use s32k144evb::{console, led, pcc, pcc::Pcc, spc};

#[path = "utils.rs"]
mod utils;

#[inline(never)]
#[panic_handler]
fn panic(info: &PanicInfo) -> ! {
    // This function is diverging, so if any settings have been previously made we can mess with
    // them freely.

    // Ensure LPUART1 is enabled
    let pcc = Pcc::init(unsafe { &*s32k144::PCC::ptr() });
    let _pcc_lpuart1 = pcc.enable_lpuart1(pcc::ClockSource::Soscdiv2);
    let _pcc_portc = pcc.enable_portc();

    let spc_config = spc::Config {
        system_oscillator: spc::SystemOscillatorInput::Crystal(8_000_000),
        soscdiv2: spc::SystemOscillatorOutput::Div1,
        ..Default::default()
    };

    // NOTE: required here to move console and led out of the closure below.
    let spc = unsafe {
        spc::Spc::init(
            &*s32k144::SCG::ptr(),
            &*s32k144::SMC::ptr(),
            &*s32k144::PMC::ptr(),
            spc_config,
        )
        .unwrap()
    };
    let pcc_portd = pcc::PortD {
        pcc: unsafe { &*s32k144::PCC::ptr() },
    };

    let (mut serial, led) = cortex_m::interrupt::free(|_cs| unsafe {
        let pcc = &*s32k144::PCC::ptr();
        let portc = &*s32k144::PORTC::ptr();
        let portd = &*s32k144::PORTD::ptr();

        pcc.pcc_portc.modify(|_, w| w.cgc()._1());
        pcc.pcc_portd.modify(|_, w| w.cgc()._1());

        // turn of all other muxes than the one that muxes to the OpenSDA
        portc.pcr7.modify(|_, w| w.mux()._010());
        portc.pcr9.modify(|_, w| w.mux()._000());
        portd.pcr14.modify(|_, w| w.mux()._000());

        (
            console::LpuartConsole::init(&*s32k144::LPUART1::ptr(), &spc),
            led::RgbLed::init(&*s32k144::PTD::ptr(), &portd, &pcc_portd),
        )
    });

    let mut led_on = true;

    loop {
        // Toggle yellow LED
        led.set(led_on, false, led_on);
        led_on = !led_on;

        cortex_m::interrupt::free(|_cs| writeln!(serial, "{}", info).unwrap());

        // Sleep one second
        utils::sleep(1000);
    }
}

#[exception]
fn HardFault(ef: &cortex_m_rt::ExceptionFrame) -> ! {
    unsafe {
        ptr::read_volatile(ef);
    }

    // light red LED
    unsafe {
        let pcc = Pcc::init(&*s32k144::PCC::ptr());
        let pcc_portd = pcc.enable_portd().unwrap();
        let led = led::RgbLed::init(&*s32k144::PTD::ptr(), &*s32k144::PORTD::ptr(), &pcc_portd);
        led.set(true, false, false);
    }

    utils::halt();
}
