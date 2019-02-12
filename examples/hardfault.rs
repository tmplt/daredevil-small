//! A simple example that hardfaults.
#![no_main]
#![no_std]

use core::ptr;
use cortex_m_rt::entry;
use s32k144;
use s32k144evb::wdog;

#[path = "../src/panic.rs"]
mod panic;

#[entry]
fn main() -> ! {
    let peripherals = s32k144::Peripherals::take().unwrap();

    // Disable the watchdog
    let wdog_settings = wdog::WatchdogSettings {
        enable: false,
        ..Default::default()
    };
    let _wdog = wdog::Watchdog::init(&peripherals.WDOG, wdog_settings).unwrap();

    unsafe {
        ptr::read_volatile(0xDEADBEEF as *const u32);
    }
    panic!("This should never be printed");
}
