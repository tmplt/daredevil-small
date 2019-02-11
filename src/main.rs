#![deny(unsafe_code)]
#![no_main]
#![no_std]

extern crate panic_halt;
extern crate s32k144;

use cortex_m_rt::entry;

#[entry]
fn main() -> ! {
    let _x = 42;

    loop {}
}
