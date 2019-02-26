//! This module is an ad-hoc implementation for encryption and decryption of 128 bits of data
//! (`&[u8; 16]`), derived from `refs/security_pal`.
//!
//! This module will eventually be able to encrypt slices `&[u8]` of arbitrary sizes.
#![allow(dead_code)]

use core::mem::transmute;
use s32k144;

mod utils;

/// CSEc commands which follow the same values as the SHE command defenition.
enum Command {
    EncEcb = 0x01,
    EncCbc,
    DecEcb,
    DecCbc,
    GenerateMac,
    VerifyMac,
    LoadKey,
    LoadPlainKey,
    ExportRamKey,
    InitRng = 0xa,
    ExtendSeed,
    Rng = 0xc,
    Reserved1,
    BootFailure,
    BootOk,
    GetId,
    BootDefine,
    DbgChal,
    DbgAuth,
    Reserved2,
    Reserved3,
    MPCompress,
}

/// Specifies how the data is transferred to/from the CSE.
/// There are two use cases. One is to copy all data and the command function call method and the
/// other is a pointer and function call method.
enum Format {
    Copy = 0x0,
    Pointer,
}

/// Specifies if the information is the first of a following function call.
enum Sequence {
    First = 0x0,
    Subsequent,
}

/// Specify the KeyID to be used to implement the requested cryptographic operation.
enum KeyID {
    SecretKey = 0x0,
    MasterEcu,
    BootMacKey,
    BootMac,
    Key1,
    Key2,
    Key3,
    Key4,
    Key5,
    Key6,
    Key7,
    Key8,
    Key9,
    Key10,
    RamKey = 0xF,
    Key11 = 0x14,
    Key12,
    Key13,
    Key14,
    Key15,
    Key16,
    Key17,
}

/// Represents the result of the execution of a command. Provides one bit for each error code as
/// per SHE specification.
#[derive(Debug)]
pub enum CommandResult {
    NoError = 0x1,
    SequenceError = 0x2,
    KeyNotAvailable = 0x4,
    KeyInvalid = 0x8,
    KeyEmpty = 0x10,
    NoSecureBoot = 0x20,
    KeyWriteProtected = 0x40,
    KeyUpdateError = 0x80,
    RngSeed = 0x100,
    NoDebugging = 0x200,
    MemoryFailure = 0x400,
    GeneralError = 0x800,
}

impl CommandResult {
    fn from_u16(value: u16) -> CommandResult {
        match value {
            0x1 => CommandResult::NoError,
            0x2 => CommandResult::SequenceError,
            0x4 => CommandResult::KeyNotAvailable,
            0x8 => CommandResult::KeyInvalid,
            0x10 => CommandResult::KeyEmpty,
            0x20 => CommandResult::NoSecureBoot,
            0x40 => CommandResult::KeyWriteProtected,
            0x80 => CommandResult::KeyUpdateError,
            0x100 => CommandResult::RngSeed,
            0x200 => CommandResult::NoDebugging,
            0x400 => CommandResult::MemoryFailure,
            0x800 => CommandResult::GeneralError,
            _ => panic!("Unknown CommandResult value: {}", value),
        }
    }
}

pub struct CSEc<'a> {
    ftfc: &'a s32k144::FTFC,
    cse_pram: &'a s32k144::CSE_PRAM,
}

const PAGE_1_OFFSET: usize = 16;
const PAGE_2_OFFSET: usize = 32;
const PAGE_LENGTH_OFFSET: usize = 14;
const PAGE_SIZE_IN_BYTES: usize = 16;

impl<'a> CSEc<'a> {
    pub fn init(
        ftfc: &'a s32k144::FTFC,
        cse_pram: &'a s32k144::CSE_PRAM,
        nvic: &mut s32k144::NVIC,
    ) -> Self {
        nvic.enable(s32k144::Interrupt::FTFC);
        utils::sleep(1);

        CSEc {
            ftfc: ftfc,
            cse_pram: cse_pram,
        }
    }

    /// Initializes the seed and derive a key for the PRNG.
    /// This function must be called before `generate_rnd`
    pub fn init_rng(&self) -> Result<(), CommandResult> {
        self.write_command_header(
            Command::InitRng,
            Format::Copy,
            Sequence::First,
            KeyID::SecretKey,
        )
    }

    /// Generates a vector of 128 random bits.
    /// This function must be called after `init_rng`.
    pub fn generate_rnd(&self, buf: &mut [u8; 16]) -> Result<(), CommandResult> {
        self.write_command_header(
            Command::Rng,
            Format::Copy,
            Sequence::First,
            KeyID::SecretKey,
        )?;

        // Read the resulted random bytes
        self.read_command_bytes(PAGE_1_OFFSET, buf, PAGE_SIZE_IN_BYTES);

        Ok(())
    }

    /// Updates the RAM key memory slot with a 128-bit plaintext.
    pub fn load_plainkey(&self, key: &[u8; 16]) -> Result<(), CommandResult> {
        // Write the bytes of the key
        self.write_command_bytes(PAGE_1_OFFSET, &key[..], PAGE_SIZE_IN_BYTES);

        self.write_command_header(
            Command::LoadPlainKey,
            Format::Copy,
            Sequence::First,
            KeyID::RamKey,
        )
    }

    /// Perform AES-128 encryption in CBC mode of the input plain text buffer.
    pub fn encrypt_cbc(
        &self,
        inbuf: &[u8],
        iv: &[u8],
        outbuf: &mut [u8; 16],
        length: usize,
    ) -> Result<(), CommandResult> {
        // Write the initialization vector
        self.write_command_bytes(PAGE_1_OFFSET, &iv, PAGE_SIZE_IN_BYTES);

        // Write the plain text
        self.write_command_bytes(PAGE_2_OFFSET, &inbuf, length);

        // Write the size of the plain/cipher text (in pages)
        // XXX: very very ad-hoc
        // self.write_command_halfword(PAGE_LENGTH_OFFSET, (16 >> 4) as u16);
        self.write_page(PAGE_LENGTH_OFFSET >> 2, &[0, 0, 0, 1]);

        self.write_command_header(
            Command::EncCbc,
            Format::Copy,
            Sequence::First,
            KeyID::RamKey,
        )?;

        self.read_command_bytes(PAGE_2_OFFSET, outbuf, length);
        Ok(())
    }

    /// Perform AES-128 decryption in CBC mode of the input cipher text buffer.
    pub fn decrypt_cbc(
        &self,
        inbuf: &[u8],
        iv: &[u8],
        outbuf: &mut [u8; 16],
        length: usize,
    ) -> Result<(), CommandResult> {
        // Write the initialization vector
        self.write_command_bytes(PAGE_1_OFFSET, &iv, PAGE_SIZE_IN_BYTES);

        // Write the cipher text
        self.write_command_bytes(PAGE_2_OFFSET, &inbuf, length);

        // Write the size of the plain/cipher text (in pages)
        // XXX: very very ad-hoc
        // self.write_command_halfword(PAGE_LENGTH_OFFSET, (16 >> 4) as u16);
        self.write_page(PAGE_LENGTH_OFFSET >> 2, &[0, 0, 0, 1]);

        self.write_command_header(
            Command::DecCbc,
            Format::Copy,
            Sequence::First,
            KeyID::RamKey,
        )?;

        self.read_command_bytes(PAGE_2_OFFSET, outbuf, length);
        Ok(())
    }

    /// Writes a command half word to `CSE_PRAM` at a 16-bit aligned offset.
    /// XXX: this function is untested!
    fn write_command_halfword(&self, offset: usize, halfword: u16) {
        let page = self.read_page(offset >> 2);
        let mut page: u32 = ((page[0] as u32) << 24)
            + ((page[1] as u32) << 16)
            + ((page[2] as u32) << 8)
            + ((page[3] as u32) << 0);

        assert!((offset & 2) != 0);

        page &= !(0xffff);
        page |= ((halfword as u32) << 0) & 0xffff;

        self.write_page(offset >> 2, unsafe { transmute(page.to_be()) });
    }

    /// Writes the command header to CSE_PRAM, triggering the CSEc operation.
    /// Waits until the operation has finished.
    fn write_command_header(
        &self,
        cmd: Command,
        cmd_format: Format,
        callseq: Sequence,
        key: KeyID,
    ) -> Result<(), CommandResult> {
        #[rustfmt::skip]
        self.cse_pram.embedded_ram0.write(|w| unsafe {
            w.byte_0().bits(cmd as u8)
                .byte_1().bits(cmd_format as u8)
                .byte_2().bits(callseq as u8)
                .byte_3().bits(key as u8)
        });
        self.wait_command_completion();

        let retval = CommandResult::from_u16(self.read_error_bits());
        match retval {
            CommandResult::NoError => Ok(()),
            _ => Err(retval),
        }
    }

    /// Waits for the completion of a CSEc command.
    fn wait_command_completion(&self) {
        while self.ftfc.fstat.read().ccif().bit_is_clear() {}
    }

    /// Reads the status of the finished operation.
    /// XXX: ad-hoc!
    fn read_error_bits(&self) -> u16 {
        // Error bits are located in the upper half word.
        let halfword = ((self.cse_pram.embedded_ram1.read().bits() & 0xFFFF0000) >> 0x10) as u16;
        return halfword;
    }

    fn read_page(&self, n: usize) -> [u8; 4] {
        let page = match n {
            0 => self.cse_pram.embedded_ram0.read().bits(),
            1 => self.cse_pram.embedded_ram1.read().bits(),
            2 => self.cse_pram.embedded_ram2.read().bits(),
            3 => self.cse_pram.embedded_ram3.read().bits(),
            4 => self.cse_pram.embedded_ram4.read().bits(),
            5 => self.cse_pram.embedded_ram5.read().bits(),
            6 => self.cse_pram.embedded_ram6.read().bits(),
            7 => self.cse_pram.embedded_ram7.read().bits(),
            8 => self.cse_pram.embedded_ram8.read().bits(),
            9 => self.cse_pram.embedded_ram9.read().bits(),
            10 => self.cse_pram.embedded_ram10.read().bits(),
            11 => self.cse_pram.embedded_ram11.read().bits(),
            12 => self.cse_pram.embedded_ram12.read().bits(),
            13 => self.cse_pram.embedded_ram13.read().bits(),
            14 => self.cse_pram.embedded_ram14.read().bits(),
            15 => self.cse_pram.embedded_ram15.read().bits(),
            16 => self.cse_pram.embedded_ram16.read().bits(),
            17 => self.cse_pram.embedded_ram17.read().bits(),
            18 => self.cse_pram.embedded_ram18.read().bits(),
            19 => self.cse_pram.embedded_ram19.read().bits(),
            20 => self.cse_pram.embedded_ram20.read().bits(),
            21 => self.cse_pram.embedded_ram21.read().bits(),
            22 => self.cse_pram.embedded_ram22.read().bits(),
            23 => self.cse_pram.embedded_ram23.read().bits(),
            24 => self.cse_pram.embedded_ram24.read().bits(),
            25 => self.cse_pram.embedded_ram25.read().bits(),
            26 => self.cse_pram.embedded_ram26.read().bits(),
            27 => self.cse_pram.embedded_ram27.read().bits(),
            28 => self.cse_pram.embedded_ram28.read().bits(),
            29 => self.cse_pram.embedded_ram29.read().bits(),
            30 => self.cse_pram.embedded_ram30.read().bits(),
            31 => self.cse_pram.embedded_ram31.read().bits(),
            _ => panic!(), // absurd
        };

        unsafe { transmute(page.to_be()) }
    }

    // make this a get page instead? get as &mut.
    fn write_page(&self, n: usize, buf: &[u8; 4]) {
        // Big-Endian
        let bytes = ((buf[0] as u32) << 24)
            + ((buf[1] as u32) << 16)
            + ((buf[2] as u32) << 8)
            + ((buf[3] as u32) << 0);

        #[rustfmt::skip]
        match n {
            0 => self.cse_pram.embedded_ram0.write(|w| unsafe { w.bits(bytes) }),
            1 => self.cse_pram.embedded_ram1.write(|w| unsafe { w.bits(bytes) }),
            2 => self.cse_pram.embedded_ram2.write(|w| unsafe { w.bits(bytes) }),
            3 => self.cse_pram.embedded_ram3.write(|w| unsafe { w.bits(bytes) }),
            4 => self.cse_pram.embedded_ram4.write(|w| unsafe { w.bits(bytes) }),
            5 => self.cse_pram.embedded_ram5.write(|w| unsafe { w.bits(bytes) }),
            6 => self.cse_pram.embedded_ram6.write(|w| unsafe { w.bits(bytes) }),
            7 => self.cse_pram.embedded_ram7.write(|w| unsafe { w.bits(bytes) }),
            8 => self.cse_pram.embedded_ram8.write(|w| unsafe { w.bits(bytes) }),
            9 => self.cse_pram.embedded_ram9.write(|w| unsafe { w.bits(bytes) }),
            10 => self.cse_pram.embedded_ram10.write(|w| unsafe { w.bits(bytes) }),
            11 => self.cse_pram.embedded_ram11.write(|w| unsafe { w.bits(bytes) }),
            12 => self.cse_pram.embedded_ram12.write(|w| unsafe { w.bits(bytes) }),
            13 => self.cse_pram.embedded_ram13.write(|w| unsafe { w.bits(bytes) }),
            14 => self.cse_pram.embedded_ram14.write(|w| unsafe { w.bits(bytes) }),
            15 => self.cse_pram.embedded_ram15.write(|w| unsafe { w.bits(bytes) }),
            16 => self.cse_pram.embedded_ram16.write(|w| unsafe { w.bits(bytes) }),
            17 => self.cse_pram.embedded_ram17.write(|w| unsafe { w.bits(bytes) }),
            18 => self.cse_pram.embedded_ram18.write(|w| unsafe { w.bits(bytes) }),
            19 => self.cse_pram.embedded_ram19.write(|w| unsafe { w.bits(bytes) }),
            20 => self.cse_pram.embedded_ram20.write(|w| unsafe { w.bits(bytes) }),
            21 => self.cse_pram.embedded_ram21.write(|w| unsafe { w.bits(bytes) }),
            22 => self.cse_pram.embedded_ram22.write(|w| unsafe { w.bits(bytes) }),
            23 => self.cse_pram.embedded_ram23.write(|w| unsafe { w.bits(bytes) }),
            24 => self.cse_pram.embedded_ram24.write(|w| unsafe { w.bits(bytes) }),
            25 => self.cse_pram.embedded_ram25.write(|w| unsafe { w.bits(bytes) }),
            26 => self.cse_pram.embedded_ram26.write(|w| unsafe { w.bits(bytes) }),
            27 => self.cse_pram.embedded_ram27.write(|w| unsafe { w.bits(bytes) }),
            28 => self.cse_pram.embedded_ram28.write(|w| unsafe { w.bits(bytes) }),
            29 => self.cse_pram.embedded_ram29.write(|w| unsafe { w.bits(bytes) }),
            30 => self.cse_pram.embedded_ram30.write(|w| unsafe { w.bits(bytes) }),
            31 => self.cse_pram.embedded_ram31.write(|w| unsafe { w.bits(bytes) }),
            _ => panic!(),  // absurd
        };
    }

    /// Reads command bytes from CSE_PRAM from a 32-bit aligned offset.
    /// XXX: ad-hoc for
    fn read_command_bytes(&self, offset: usize, buf: &mut [u8; 16], bytes: usize) {
        let mut i = 0;
        while (i + 3) < bytes {
            // NOTE(arg): Translate offset to page number.
            let page = self.read_page((offset + i) >> 2);

            // buf[i..i+3] = page[0..3];

            buf[i] = page[0];
            buf[i + 1] = page[1];
            buf[i + 2] = page[2];
            buf[i + 3] = page[3];
            i += 4;
        }

        while i < bytes {
            buf[i] = self.read_command_byte(offset + i);
            i += 1;
        }
    }

    fn write_command_bytes(&self, offset: usize, buf: &[u8], bytes: usize) {
        let mut i = 0;
        while (i + 3) < bytes {
            // XXX: crash here
            self.write_page(
                (offset + i) >> 2,
                &[buf[i], buf[i + 1], buf[i + 2], buf[i + 3]],
            );
            i += 4;
        }

        while i < bytes {
            self.write_command_byte(offset + i, buf[i]);
            i += 1;
        }
    }

    /// Reads a single byte from `CSE_PRAM`.
    fn read_command_byte(&self, offset: usize) -> u8 {
        let page = self.read_page(offset >> 2);

        match offset & 0x3 {
            0x0 => page[3], // HU
            0x1 => page[2], // HL
            0x2 => page[1], // LU
            0x3 => page[0], // LL
            _ => panic!(),  // absurd
        }
    }

    /// Writes a single byte from `CSE_PRAM`.
    fn write_command_byte(&self, offset: usize, byte: u8) {
        let page = self.read_page(offset >> 2);
        let page: [u8; 4] = match offset & 0x3 {
            0x0 => [byte, page[1], page[2], page[3]], // HU
            0x1 => [page[0], byte, page[2], page[3]], // HL
            0x2 => [page[0], page[1], byte, page[3]], // LU
            0x3 => [page[0], page[1], page[3], byte], // LL
            _ => panic!(),                            // absurd
        };

        self.write_page(offset >> 2, &page)
    }
}
