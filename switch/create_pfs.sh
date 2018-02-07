#!/usr/bin/bash

elf2nso bochs.elf
mkdir -p exefs
cp bochs.nso exefs/main
build_pfs0 exefs bochs.pfs0
