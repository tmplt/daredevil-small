#![deny(unsafe_code)]
#![no_main]
#![no_std]

// Define a panic handler.
extern crate panic_halt;

use cortex_m_rt::entry;

#[entry]
fn main() -> ! {
    let _x = 42;

    loop {}
}
