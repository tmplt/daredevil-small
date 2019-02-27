//! wow
#![no_main]
#![no_std]

extern crate s32k144;
mod csec;
mod panic;
mod utils;

use cortex_m_rt::entry;

#[entry]
fn main() -> ! {
    let _x = 42;

    loop {}
}
