# Daredevil-small

This repository constitutes the sources for the sensor array module of Daredevil (the *light/small* EVITA module),
a [S32K144EVB-Q100 MCU](https://www.nxp.com/support/developer-resources/evaluation-and-development-boards/analog-toolbox/s32k144-evaluation-board:S32K144EVB) (microcontroller unit) that read ranges from ultrasonic sensors and propagate that data over CAN-FD to an external entity.

## Documentation
All code in this repository is thoroughly documented, including examples.
Run `cargo doc [--no-deps] --document-private-items --open` to build and open all relevant documentation in your web browser.
Crate documentation presumes that this README has been read.

MCU documentation can be found in [the documentation directory](./doc).

## Dependencies & Prerequisites

### Hardware
For every sensor array module, the following is needed:
* An NXP S32K144EVB-Q100 MCU;
* 4x [MaxBotix LV-MaxSonar-EZ1](https://www.maxbotix.com/Ultrasonic_Sensors/MB1010.htm);
* a female D-Sub to four-pin CAN adapter (or some Dupont wires), and
* a barrel power supply with 12V yield.

### Software
This crate expects your MCU to be flashed with a certain debug application.
Please refer to [the flashing documentation](./doc/FLASHING.md) before continuing.

A recent stable Rust tool chain is required (v1.32.0 or later).
Install `rustup` via your system's preferred channels or by executing `curl https://sh.rustup.rs -sSf | sh` in a shell instance.
Then run
```
$ rustup default stable
$ rustup update stable
$ rustup target add thumbv7em-none-eabihf # required to compile to the ARM Cortex-M4
$ cargo install bobbin-cli # convenience tool for working with the S32K144EVB
```

Additionally, SEGGER's software debugger JLink is required to program your application onto the MCU.
Some other Rust-external tools are required.
Install them via:

#### Debian and derivatives (Ubuntu, [et al.](https://en.wikipedia.org/wiki/List_of_Linux_distributions#Debian-based))
```
# curl -d accept_license_agreement=accepted -d confirm=yes https://www.segger.com/downloads/jlink/JLink_Linux_x86_64.deb | dpkg --install -
# apt install gdb-multiarch
```

#### Arch Linux
```
$ git clone https://aur.archlinux.org/jlink-software-and-documentation.git
$ cd jlink-software-and-documentation
$ makepkg -cs
$ pacman -U *.pkg.tar.xz
```
or use your AUR helper of choice.

And:

```
# pacman -Syu arm-none-eabi-gdb
```

#### Nix(OS)
```
$ nix-shell shell.nix
```

## Deployment

1. Connect the MCU to your system via USB.
2. Connect the power source to the MCU (if CAN functionality is wanted).
3. Start a GDB server by executing `./gdbserver.sh` in a separate shell instance.
4. Program your application via `cargo run [--example <example-name>]`.

The target application will be flashed automatically (see `./gdbinit`), and a breakpoint will be installed at the start of `main`.

If you wish to attach to the UART console, run `bobbin-cli console`.

For instructions on how to connect sensors and read CAN output, please refer to the [overview documentation](https://gitlab.com/rust-daredevil-group/daredevil#how-to-get-started).

## License
Licensed under either of

- Apache License, Version 2.0 ([LICENSE-APACHE](LICENSE-APACHE) or http://www.apache.org/licenses/LICENSE-2.0)
- MIT license ([LICENSE-MIT](LICENSE-MIT) or http://opensource.org/licenses/MIT)

at your option.
