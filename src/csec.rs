//! This module is an ad-hoc implementation for encryption and decryption of 128 bits of data
//! (`&[u8; 16]`), derived from `refs/security_pal`.
//!
//! This module will eventually be able to encrypt slices `&[u8]` of arbitrary sizes.
#![allow(dead_code)]

use core::mem::transmute;
use s32k144;

mod utils;

/// CSEc commands which follow the same values as the SHE command defenition.
#[derive(Debug, Clone, Copy)]
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
    InitRng,
    ExtendSeed,
    Rng,
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
#[derive(PartialEq)]
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
    RamKey = 0xf,
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
const ERROR_BITS_OFFSET: usize = 4;
const LOWER_HALF_MASK: u32 = 0xffff;
const LOWER_HALF_SHIFT: u32 = 0x0;
const UPPER_HALF_MASK: u32 = 0xffff0000;
const UPPER_HALF_SHIFT: u32 = 0x10;
const BYTES_TO_PAGES_SHIFT: u32 = 4;
const MAX_PAGES: usize = 7;

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
    /// This function must be called before `generate_rnd`.
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
    pub fn load_plainkey(&self, key: &[u8; PAGE_SIZE_IN_BYTES]) -> Result<(), CommandResult> {
        // Write the bytes of the key
        self.write_command_bytes(PAGE_1_OFFSET, key, key.len());

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
        plaintext: &[u8],
        iv: &[u8; PAGE_SIZE_IN_BYTES],
        ciphertext: &mut [u8],
    ) -> Result<(), CommandResult> {
        self.handle_cbc(Command::EncCbc, plaintext, iv, ciphertext)
    }

    /// Perform AES-128 decryption in CBC mode of the input cipher text buffer.
    pub fn decrypt_cbc(
        &self,
        ciphertext: &[u8],
        iv: &[u8; PAGE_SIZE_IN_BYTES],
        plaintext: &mut [u8],
    ) -> Result<(), CommandResult> {
        self.handle_cbc(Command::DecCbc, ciphertext, iv, plaintext)
    }

    fn handle_cbc(
        &self,
        command: Command,
        input: &[u8],
        iv: &[u8; PAGE_SIZE_IN_BYTES],
        output: &mut [u8],
    ) -> Result<(), CommandResult> {
        assert!(output.len() >= input.len());

        // Write the initialization vector and how many pages we are going to proccess
        self.write_command_bytes(PAGE_1_OFFSET, &iv[..], iv.len());
        self.write_command_halfword(
            PAGE_LENGTH_OFFSET,
            (input.len() >> BYTES_TO_PAGES_SHIFT) as u16,
        );

        fn process_blocks(
            cse: &CSEc,
            input: &[u8],
            output: &mut [u8],
            sequence: Sequence,
            command: Command,
        ) -> Result<(), CommandResult> {
            // On first call page 1 is occupied by the initialization vector, so we have one less.
            // On Subsequent calls we have all at our disposal.
            let (page_offset, avail_pages) = if sequence == Sequence::First {
                (PAGE_2_OFFSET, MAX_PAGES - 1)
            } else {
                (PAGE_1_OFFSET, MAX_PAGES)
            };

            // How many bytes are we processing this round?
            let bytes = core::cmp::min(input.len(), avail_pages * PAGE_SIZE_IN_BYTES);

            cse.write_command_bytes(page_offset, &input[..bytes], bytes);
            cse.write_command_header(command, Format::Copy, sequence, KeyID::RamKey)?;
            cse.read_command_bytes(page_offset, &mut output[..bytes], bytes);

            // Process remaining blocks, if any
            if input.len() - bytes != 0 {
                process_blocks(
                    cse,
                    &input[bytes..],
                    &mut output[bytes..],
                    Sequence::Subsequent,
                    command,
                )
            } else {
                Ok(())
            }
        }

        process_blocks(self, input, output, Sequence::First, command)
    }

    /// Writes the command header to `CSE_PRAM`, triggering the CSEc operation.
    /// Blocks until the operation has finished.
    fn write_command_header(
        &self,
        cmd: Command,
        cmd_format: Format,
        callseq: Sequence,
        key: KeyID,
    ) -> Result<(), CommandResult> {
        match cmd {
            Command::InitRng
            | Command::Rng
            | Command::LoadPlainKey
            | Command::EncCbc
            | Command::DecCbc => (),
            _ => unimplemented!("Command {:?}", cmd),
        };

        #[rustfmt::skip]
        self.cse_pram.embedded_ram0.write(|w| unsafe {
            w.byte_0().bits(cmd as u8)
                .byte_1().bits(cmd_format as u8)
                .byte_2().bits(callseq as u8)
                .byte_3().bits(key as u8)
        });

        // Wait until the operation has finished
        while self.ftfc.fstat.read().ccif().bit_is_clear() {}

        let status = CommandResult::from_u16(self.read_command_halfword(ERROR_BITS_OFFSET));
        match status {
            CommandResult::NoError => Ok(()),
            _ => Err(status),
        }
    }

    /// Reads a command half word from `CSE_PRAM` from a 16-bit aligned offset.
    fn read_command_halfword(&self, offset: usize) -> u16 {
        let page = self.read_pram(offset >> 2);
        let halfword: [u8; 2] = match (offset & 2) != 0 {
            true => [page[2], page[3]],
            false => [page[0], page[1]],
        };

        u16::from_be_bytes(halfword)
    }

    /// Writes a command half word to `CSE_PRAM` at a 16-bit aligned offset.
    fn write_command_halfword(&self, offset: usize, halfword: u16) {
        let mut page = u32::from_be_bytes(self.read_pram(offset >> 2));
        if (offset & 2) != 0 {
            page &= !LOWER_HALF_MASK;
            page |= ((halfword as u32) << LOWER_HALF_SHIFT) & LOWER_HALF_MASK;
        } else {
            page &= !UPPER_HALF_MASK;
            page |= ((halfword as u32) << LOWER_HALF_SHIFT) & UPPER_HALF_MASK;
        }

        let newpage: [u8; 4] = unsafe { transmute(page.to_be()) };
        self.write_pram(offset >> 2, &newpage);
    }

    /// Reads a single byte from `CSE_PRAM`.
    fn read_command_byte(&self, offset: usize) -> u8 {
        let page = self.read_pram(offset >> 2);

        match offset & 0x3 {
            0x0 => page[0], // LL
            0x1 => page[1], // LU
            0x2 => page[2], // HL
            0x3 => page[3], // HU
            _ => panic!(),  // absurd
        }
    }

    /// Writes a single byte from `CSE_PRAM`.
    fn write_command_byte(&self, offset: usize, byte: u8) {
        let page = self.read_pram(offset >> 2);
        let page: [u8; 4] = match offset & 0x3 {
            0x0 => [byte, page[1], page[2], page[3]], // LL
            0x1 => [page[0], byte, page[2], page[3]], // LU
            0x2 => [page[0], page[1], byte, page[3]], // HL
            0x3 => [page[0], page[1], page[2], byte], // HU
            _ => panic!(),                            // absurd
        };

        self.write_pram(offset >> 2, &page)
    }

    /// Reads command bytes from `CSE_PRAM` from a 32-bit aligned offset.
    fn read_command_bytes(&self, offset: usize, buf: &mut [u8], bytes: usize) {
        assert!(buf.len() >= bytes);

        let mut i = 0;
        while (i + 3) < bytes {
            let page = self.read_pram((offset + i) >> 2);

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

    /// Writes command bytes from `CSE_PRAM` from a 32-bit aligned offset.
    fn write_command_bytes(&self, offset: usize, buf: &[u8], bytes: usize) {
        assert!(buf.len() >= bytes);

        let mut i = 0;
        while (i + 3) < bytes {
            self.write_pram(
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

    fn read_pram(&self, n: usize) -> [u8; 4] {
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

    fn write_pram(&self, n: usize, buf: &[u8; 4]) {
        let bytes = u32::from_be_bytes(*buf);

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
}
