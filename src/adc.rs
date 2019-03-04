//! Ad-hoc ADC implementation

use s32k144;

const MAGIC_SCALAR: u16 = 5000 / 0xfff;

pub struct ADC {
    adc: s32k144::ADC0,
}

impl ADC {
    pub fn init(pcc: &s32k144::PCC, adc: s32k144::ADC0) -> Self {
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

        ADC { adc: adc }
    }

    pub fn read(&self) -> u16 {
        // Configure ADC0 channel 15, ADC0_SE15. Translates to pin PTC17.
        // Must be done every loop; allows us to read from the channel.
        self.adc.sc1a.modify(|_, w| w.adch()._00000());
        self.adc.sc1a.write(|w| unsafe { w.bits(15) });

        // Read sensor value.
        while self.adc.sc1a.read().coco().bit_is_set() == true {} // wait while clock is changed
        MAGIC_SCALAR * self.adc.ra.read().bits() as u16
    }
}
