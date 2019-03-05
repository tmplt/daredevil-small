//! Ad-hoc ADC implementation

use crate::utils;
use s32k144;

/// ADC0_SE15
const PTC17: u8 = 15;

/// ADC0_SE14
const PTC16: u8 = 14;

/// ADC0_SE13
const PTC15: u8 = 13;

/// ADC0_SE9
const PTC1: u8 = 9;

/// Scales read ADC value to 5000 mW
const MW_SCALAR: f32 = 12.6 / 4.0;

pub const CHANNEL_COUNT: usize = 4;
const CHANNELS: [u8; CHANNEL_COUNT] = [PTC17, PTC16, PTC15, PTC1];

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

    /// Read the values of all four sensors.
    pub fn read(&self) -> [u16; CHANNEL_COUNT] {
        let mut sensor_values: [u16; CHANNEL_COUNT] = [0; CHANNEL_COUNT];
        for i in 0..CHANNEL_COUNT {
            sensor_values[i] = self.read_adc(i);
        }

        sensor_values
    }

    /// Reads a single ADC channel, scaling its value to 5000 mW.
    fn read_adc(&self, channel: usize) -> u16 {
        // Required software trigger to read from ADC channel.
        self.adc.sc1a.modify(|_, w| w.adch()._00000());
        self.adc
            .sc1a
            .write(|w| unsafe { w.bits(CHANNELS[channel] as u32) });
        while self.adc.sc1a.read().coco().bit_is_set() {} // wait for conversion

        let value = ((self.adc.ra.read().bits() as f32) * MW_SCALAR) as u16;

        // Necessary due to the previous sensor triggering the next in hardware (connected in serial); the MCU must not
        // read the next sensor until a range has been recorded.
        //
        // TODO: move this out of this function
        utils::sleep(10);

        value
    }
}
