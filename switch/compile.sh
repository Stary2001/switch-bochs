#!/bin/bash
export CC=${DEVKITA64}/bin/aarch64-none-elf-gcc
export CXX=${DEVKITA64}/bin/aarch64-none-elf-g++
export LD=${DEVKITA64}/bin/aarch64-none-elf-ld

SHARED_FLAGS="-Wall -O0 -fomit-frame-pointer -ffunction-sections -march=armv8-a -mtp=soft -fPIE -g -DSWITCH=1"
export CFLAGS="$SHARED_FLAGS -I${DEVKITPRO}/libnx/include -lnx"
export CXXFLAGS="$SHARED_FLAGS -I${DEVKITPRO}/libnx/include -fno-rtti -fno-exceptions -std=gnu++11"
export LDFLAGS="$SHARED_FLAGS -L${DEVKITPRO}/libnx/lib -specs=switch.specs -lnx"

# ./configure --with-switch --disable-shared --enable-static --host=aarch64-elf --disable-show-ips --enable-all-optimizations
