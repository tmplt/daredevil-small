#!/usr/bin/env bash
# TODO: mass erase device and program examples/nxp-flexcanfd.rs with this.
JLinkGDBServerCLExe -if jtag -device S32K144 -endian little -speed 1000 -port 2331 -swoport 2332 -telnetport 2333 -vd -ir -localhostonly 1 -strict -timeout 0 -nogui
