# Daredevil
This repository constitutes the sources for the sensor array module of Daredevil,
a S32K144 MCU that will read ranges from ultrasonic sensors and propagate that data over CAN to an external entity.

### Prerequisites
* Your S32K144 MCU has been flashed as described by [the flashing documentation](./doc/FLASHING.md).

### Install
The following packages need to be installed to do stuff with the S32K144EVB.

#### SEGGER's J-link
Install SEGGER's J-link package to get the GDB server to interface over J-link.

#### Debian
`https://www.segger.com/downloads/jlink/JLink_Linux_x86_64.deb`

#### Arch
* [https://aur.archlinux.org/packages/jlink/]([https://aur.archlinux.org/packages/jlink/]) which depends on:

```
jlink-software-and-documentation
jlink-systemview
ozone
```

### `arm-none-eabi-gdb`

### `arm-none-eabi-binutils`


#### Rust stuff
Install `rustup`, add default toolchain as stable and add the appropriate target system.

```
$ curl https://sh.rustup.rs -sSf | sh
$ rustup default stable
$ rustup target add thumbv7em-none-eabihf
```

Install `bobbin-cli` to simplify speaking with the MCU via serial/UART.

```
cargo install bobbin-cli
```

### Running
1. Start the J-link GDB server:
`JLinkGDBServerCLExe -select USB -device S32K144 -if SWD -speed 4000 -ir`

2. To build the project to the target architecture you have to ensure the `./.cargo/config` file is
correct, e.g:
```
[target.thumbv7em-none-eabihf]
runner = "arm-none-eabi-gdb -q -x .gdbinit"
rustflags = [
  "-C", "link-arg=-Tlink.x",
]

[build]
target = "thumbv7em-none-eabihf"
```

Then run:
```
$ cargo run
```

**NOTE:** If you do not want to use `cargo run`, do the following:

* Build the project: `cargo build`.

* Run `arm-none-eabi-gdb` with the appropriate options (assuming you are in the base of this
project's directory structure):
`arm-none-eabi-gdb -x .gdbinit target/thumb7em-none-eabihf/debug/daredevil-small`

3. To get UART output run:

```
$ bobbin-cli console
```

### Troubleshooting
#### Permission denied when using `bobbin-cli console`
Try running it as `sudo`:

```
$ sudo bobbin-cli console
```
