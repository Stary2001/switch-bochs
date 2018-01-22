#!/bin/bash
export CC=${DEVKITA64}/bin/aarch64-none-elf-gcc
export CXX=${DEVKITA64}/bin/aarch64-none-elf-g++

SHARED_FLAGS="-Wall -O0 -mword-relocations -fomit-frame-pointer -ffunction-sections -march=armv8-a -mtp=soft -fPIE -g"
export CFLAGS="$SHARED_FLAGS -I${DEVKITPRO}/libnx/include"
export CXXFLAGS="$SHARED_FLAGS -fno-rtti -fno-exceptions -std=gnu++11"
export LDFLAGS="$SHARED_FLAGS -L${DEVKITPRO}/libnx/lib -specs=switch.specs"

# ./configure --with-switch --disable-shared --enable-static --host=aarch64-none-elf --disable-show-ips --enable-all-optimizations
