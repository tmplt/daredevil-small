#![no_main]
#![no_std]

// Define a panic handler.
extern crate panic_halt;
extern crate s32k144;

use cortex_m_rt::entry;

const RED_PIN: u32 = 15;
const BLUE_PIN: u32 = 0;
const GREEN_PIN: u32 = 16;

fn led_set(ptd: &s32k144::ptd::RegisterBlock, red: bool, blue: bool, green: bool) {
    if red {
        ptd.pcor.write(|w| unsafe { w.ptco().bits(1 << RED_PIN) });
    } else {
        ptd.psor.write(|w| unsafe { w.ptso().bits(1 << RED_PIN) });
    }

    if green {
        ptd.pcor.write(|w| unsafe { w.ptco().bits(1 << GREEN_PIN) });
    } else {
        ptd.psor.write(|w| unsafe { w.ptso().bits(1 << GREEN_PIN) });
    }

    if blue {
        ptd.pcor.write(|w| unsafe { w.ptco().bits(1 << BLUE_PIN) });
    } else {
        ptd.psor.write(|w| unsafe { w.ptso().bits(1 << BLUE_PIN) });
    }
}

#[entry]
fn main() -> ! {
    let mut peripherals = s32k144::Peripherals::take().unwrap();

    // Disable the watchdog (note that all bits MUST be configured)
    unsafe {
        peripherals.WDOG.toval.write(|w| w.bits(0b0000010000000000 as u32));
        peripherals.WDOG.win.write(|w| w.bits(0x0000 as u32));
    }

    peripherals.WDOG.cs.modify(|_, w| w
                               .stop().bit(false)
                               .wait().bit(false)
                               .dbg().bit(false)
                               .update().bit(false)
                               .int().bit(false)
                               .en().bit(false)
                               .pres().bit(false)
                               .cmd32en()._1()
                               .win().bit(false)
    );

    // pcc: enable portd
    peripherals.PCC.pcc_portd.modify(|_, w| w.cgc()._1());

    // led: enable all three LEDs
    peripherals.PTD.pddr.write(|w| unsafe {
        w.pdd()
            .bits(peripherals.PTD.pddr.read().bits() | (1 << 0) | (1 << 15) | (1 << 16))
    });

    peripherals.PORTD.pcr0.modify(|_, w| w.mux().bits(0b001));
    peripherals.PORTD.pcr0.modify(|_, w| w.dse()._1());
    peripherals.PORTD.pcr0.modify(|_, w| w.pe()._0());

    peripherals.PORTD.pcr15.modify(|_, w| w.mux().bits(0b001));
    peripherals.PORTD.pcr15.modify(|_, w| w.dse()._1());
    peripherals.PORTD.pcr15.modify(|_, w| w.pe()._0());

    peripherals.PORTD.pcr16.modify(|_, w| w.mux().bits(0b001));
    peripherals.PORTD.pcr16.modify(|_, w| w.dse()._1());
    peripherals.PORTD.pcr16.modify(|_, w| w.pe()._0());

    loop {
        // XXX: Hardfault!?
        // led_set(&peripherals.PTD, true, false, false);

        let loop_max = 3000;

        for i in 0..8 * loop_max {
            match i / loop_max {
                0 => led_set(&peripherals.PTD, false, false, false),
                1 => led_set(&peripherals.PTD, false, false, true),
                2 => led_set(&peripherals.PTD, false, true, false),
                3 => led_set(&peripherals.PTD, false, true, true),
                // XXX: below statements lead to hardfaults. Only when red = true
                // 4 => led_set(&peripherals.PTD, true, false, false),
                // 5 => led_set(&peripherals.PTD, true, false, true),
                // 6 => led_set(&peripherals.PTD, true, true, false),
                // 7 => led_set(&peripherals.PTD, true, true, true),
                _ => continue,
            }
        }
    }
}
