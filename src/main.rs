//! An example that reads a sensor on pin PTC17 and dumps its value over CAN-FD, wrapped in RTFM.
#![no_main]
#![no_std]

use rtfm::{app, Instant};
use s32k144::Interrupt;
use s32k144evb::wdog;

mod panic;
mod csec;
mod utils;

const MAGIC_SCALAR: u32 = 5000 / 0xfff;
const MSG_BUF_SIZE: usize = 18;
const PERIOD: u32 = 8_000;

#[app(device = s32k144)]
const APP: () = {
    // Resources
    static mut ADC: s32k144::ADC0 = ();
    static mut CAN: s32k144::CAN0 = ();
    static mut CSEc: csec::CSEc = ();

    #[init(schedule = [poll_sensor])]
    fn init() {
        // Disable watchdog
        let wdog_settings = wdog::WatchdogSettings {
            enable: false,
            ..Default::default()
        };
        let _wdog = wdog::Watchdog::init(&device.WDOG, wdog_settings).unwrap();

        // Settings required for CAN-FD
        configure_spll_clock(&device.SCG);

        adc_init(&device.PCC, &device.ADC0);
        flexcan0_init(
            &device.PCC,
            &device.CAN0,
            &device.PORTE,
            &device.PORTD,
            &device.PTD,
        );

        let plainkey: [u8; 16] = [
            0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f,
            0x3c,
        ];

        let csec = csec::CSEc::init(device.FTFC, device.CSE_PRAM);
        csec.init_rng().unwrap();
        csec.load_plainkey(&plainkey).unwrap();

        schedule
            .poll_sensor(Instant::now() + PERIOD.cycles())
            .unwrap();

        ADC = device.ADC0;
        CAN = device.CAN0;
        CSEc = csec;
    }

    #[idle]
    fn idle() -> ! {
        // Sleep
        loop {
            rtfm::pend(Interrupt::DMA0);
        }
    }

    #[task(resources = [ADC, CAN, CSEc], schedule = [poll_sensor])]
    fn poll_sensor() {
        let adc = resources.ADC;
        let can = resources.CAN;
        let csec = resources.CSEc;

        // Configure ADC0 channel 15, ADC0_SE15. Translates to pin PTC17.
        // Must be done every loop; allows us to read from the channel.
        adc.sc1a.modify(|_, w| w.adch()._00000());
        adc.sc1a.write(|w| unsafe { w.bits(15) });

        // Read sensor value.
        while adc.sc1a.read().coco().bit_is_set() == true {} // wait while clock is changed
        let sensor_values: [u8; 2] = split_u16_to_byte_array((MAGIC_SCALAR * adc.ra.read().bits()) as u16);

        // Randomize init_vector
        const MSG_LEN: usize = 16;
        const RND_BUF_LEN: usize = 16;
        let mut rnd_buf: [u8; RND_BUF_LEN] = [0; RND_BUF_LEN];
        csec.generate_rnd(&mut rnd_buf).unwrap();

        // Encrypt data
        let mut enc_value: [u8; MSG_LEN * 10 + 7] = [0; MSG_LEN * 10 + 7];
        csec.encrypt_cbc(&sensor_values, &rnd_buf, &mut enc_value).unwrap();

        transmit_over_can(&can, &mut enc_value);

        schedule.poll_sensor(scheduled + PERIOD.cycles()).unwrap();
    }

    // Interrupt handlers used to dispatch software tasks
    extern "C" {
        fn DMA0();
    }
};


fn configure_spll_clock(scg: &s32k144::SCG) {
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

fn adc_init(pcc: &s32k144::PCC, adc: &s32k144::ADC0) {
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
}

fn flexcan0_init(
    pcc: &s32k144::PCC,
    can: &s32k144::CAN0,
    porte: &s32k144::PORTE,
    portd: &s32k144::PORTD,
    ptd: &s32k144::PTD,
) {
    unsafe {
        pcc.pcc_flex_can0.modify(|_, w| w.cgc()._1());
        can.mcr.modify(|_, w| w.mdis()._1());
        can.ctrl1.modify(|_, w| w.clksrc()._1());
        can.mcr.modify(|_, w| w.mdis()._0());
        while can.mcr.read().frzack().is_0() {}

        // Configure nominal phase
        #[rustfmt::skip]
        can.cbt.write(|w| {
            w.epseg2().bits(0b0_1111) // Bit length of phase segment 2
                .epseg1().bits(0b0_1111) // Bit length of phase segment 2
                .epropseg().bits(0b10_1110) // Bit time length of propagation segment
                .erjw().bits(0b0_1111) // Extended Resync Jump Width
                .epresdiv().bits(0b00_0000_0001) // Ratio between PE clock and Sclock
                .btf()._1() // Enable bit timing format
        });

        // Configure data phase
        #[rustfmt::skip]
        can.fdcbt.write(|w| {
            w.fpseg2().bits(0b011) // Bit time length of Fast Phase Segment 2
                .fpseg1().bits(0b111) // Bit time length of Fast Phase Segment 1
                .fpropseg().bits(0b0_0111) // Bit time length of the propagation segment
                .frjw().bits(0b011) // Number of time quanta per resynchronization
                .fpresdiv().bits(0b00_0000_0001) // Ratio between PE clock and Sclock
        });
        #[rustfmt::skip]
        can.fdctrl.write(|w| {
            w.tdcoff().bits(0b1_1111) // Transceiver Delay Compensation Offset
                .tdcen()._1() // Enable TDC
                .mbdsr0()._11() // 64 bytes per message buffer
                .fdrate()._1() // Enable Bit Rate Switch
        });

        // Clear 128 words RAM in module
        for i in 0..128 {
            can.embedded_ram[i].write(|w| w.bits(0));
        }
        // init CAN0 16 msg buf filters
        for i in 0..16 {
            can.rximr[i].write(|w| w.bits(0xFFFFFFFF));
        }

        // Global acceptance mask: check all ID bits
        can.rxmgmask.write(|w| w.bits(0x1FFFFFFF));

        // Message Buffer 4 - receive setup
        can.embedded_ram[(4 * MSG_BUF_SIZE) + 0].modify(|_, w| w.bits(0xC4000000));
        // Msg buf 4, word 1: standard ID = 0x511
        can.embedded_ram[(4 * MSG_BUF_SIZE) + 1].modify(|_, w| w.bits(0x14440000));

        // Enable CRC fix for ISO CAN FD
        can.ctrl2.modify(|_, w| w.isocanfden()._1());

        // Negate FlexCAN 1 halt state & enable CAN FD for 32 MBs
        can.mcr.write(|w| w.bits(0x0000081F));

        // Wait for FRZACK to clear and module ready
        while can.mcr.read().frzack().is_1() {}
        while !can.mcr.read().notrdy().is_0() {}
    }

    // Initialize CAN ports, and light green LED
    unsafe {
        pcc.pcc_porte.modify(|_, w| w.cgc()._1());
        porte.pcr4.modify(|_, w| w.mux()._101());
        porte.pcr5.modify(|_, w| w.mux()._101());
        pcc.pcc_portd.modify(|_, w| w.cgc()._1());
        portd.pcr16.write(|w| w.bits(0x100));
        ptd.pddr.modify(|_, w| w.bits(1 << 16));
    }
}

//fn crypto_init(ftfc: &s32k144::FTFC, cse_pram: &s32k144::CSE_PRAM, nvic: &s32k144::NVIC) -> csec::CSEc<'_> {
//    csec
//}

fn split_u16_to_byte_array(n: u16) -> [u8; 2] {
    let x: u8 = ((n >> 8) & 0xff) as u8;
    let y: u8 = (n & 0xff) as u8;
    [x, y]
}

fn byte_array_to_u32(array: &mut [u8]) -> u32 {
    let mut x: u32 = (array[3] as u32) << 24;
    x += (array[2] as u32) << 16;
    x += (array[1] as u32)  << 8;
    x += array[0] as u32;
    x
}

fn transmit_over_can(can: &s32k144::CAN0, payload: &mut [u8]) {
    // FLEXCAN0_transmit_msg
    unsafe {
        can.iflag1.write(|w| w.bits(0x1));
        let pl: u32 = byte_array_to_u32(payload);
        can.embedded_ram[(0 * MSG_BUF_SIZE) + 2].write(|w| w.bits(0xa5112233));
        can.embedded_ram[(0 * MSG_BUF_SIZE) + 3].write(|w| w.bits(pl));
        can.embedded_ram[(0 * MSG_BUF_SIZE) + 1].write(|w| w.bits(0x15540000));
        can.embedded_ram[(0 * MSG_BUF_SIZE) + 0].write(|w| w.bits(0xcc4f0000 | (8 << 16)));
    }
}
