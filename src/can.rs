//! Ad-hoc CAN-FD implementation
//!
//! Arbitration/nominal phase parameters: 500 kbit/s, 80.0%, sjw=16, tseg1=63, tseg2=16
//! Data phase parameters: 2000 kbit/s, 80.0%, sjw=4, tseg1=15, tseg2=4

use s32k144;

const MSG_BUF_SIZE: usize = 18;

pub struct CAN {
    can: s32k144::CAN0,
}

impl CAN {
    pub fn init(
        pcc: &s32k144::PCC,
        can: s32k144::CAN0,
        porte: &s32k144::PORTE,
        portd: &s32k144::PORTD,
        ptd: &s32k144::PTD,
    ) -> Self {
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

        CAN { can: can }
    }

    pub fn transmit(&self, payload: &[u8]) {
        unsafe {
            // Clear interrupt flag, sending the message
            self.can.iflag1.write(|w| w.bits(0x1));

            // Write headers?
            self.can.embedded_ram[(0 * MSG_BUF_SIZE) + 1].write(|w| w.bits(0x15540000));
            self.can.embedded_ram[(0 * MSG_BUF_SIZE) + 0].write(|w| w.bits(0xcc4f0000 | (8 << 16)));

            // Write the payload into [u8; 4] sections
            for i in 0..core::cmp::min(64, payload.len()) {
                self.can.embedded_ram[(0 * MSG_BUF_SIZE) + (i / 4)].modify(|_, w| match i % 4 {
                    0 => w.data_byte_0().bits(payload[i]),
                    1 => w.data_byte_1().bits(payload[i]),
                    2 => w.data_byte_2().bits(payload[i]),
                    3 => w.data_byte_3().bits(payload[i]),
                    _ => unreachable!(),
                });
            }
        }
    }
}
