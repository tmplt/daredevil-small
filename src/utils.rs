//! A collection of utility functions.
#![allow(dead_code)]

use core::sync::atomic::{self, Ordering};

/// Sleep for `i` milliseconds.
/// XXX: ad-hoc
/// TODO: improve this
pub fn sleep(i: u32) {
    for _ in 0..100 * i {
        atomic::compiler_fence(Ordering::SeqCst);
    }
}

pub fn halt() -> ! {
    loop {
        atomic::compiler_fence(Ordering::SeqCst);
    }
}
