//! Ad-hoc SCG configuration

use s32k144;

pub fn configure_spll_clock(scg: &s32k144::SCG) {
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
