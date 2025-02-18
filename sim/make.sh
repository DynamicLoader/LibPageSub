#!/bin/bash


opts="-O1 -nostdlib -nostartfiles -static -s -T ./sim.ld"
opts_have_libc="-O1 -nostartfiles -static -s -T ./sim.ld"

arch=$1
prefix=$2
if [ -z "$arch" ]; then
    echo "Usage: $0 <arch> [prefix]"
    exit 1
fi

mkc() {
    local file=$1
    ${prefix}gcc -DSIM_NO_LIBC $opts ./$file.c -o $arch/$file.elf
}

mkcl() {
    local file=$1
    ${prefix}gcc -DSIM_WITH_LIBC $opts_have_libc ./$file.c -o $arch/$file.elf
}

mkdir -p $arch

mkc pure_write
mkc crc32

mkcl randmem


