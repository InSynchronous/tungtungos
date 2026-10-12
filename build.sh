#!/bin/bash

set -e

mkdir -p build
mkdir -p build/kernel

echo "[1/8] Assembling bootloader..."
nasm -f bin src/kernel/boot.asm -o build/boot.bin

echo "[2/8] Compiling kernel..."
g++ -m32 \
    -Ibuild/generated\
    -ffreestanding \
    -fno-pie \
    -fno-pic \
    -fno-exceptions \
    -fno-rtti \
    -fno-stack-protector \
    -c src/kernel/ttoskernel.cpp \
    -o build/kernel/ttoskernel.o

echo "[3/8] Assembling ISR..."
nasm -f elf32 src/kernel/isr.asm \
    -o build/kernel/isr.o

echo "[4/8] Assembling Ring 3 code..."
nasm -f elf32 src/kernel/user_ring.asm \
    -o build/kernel/user_ring.o

echo "[5/8] Linking..."
ld -m elf_i386 \
    -T linker/kernel.ld \
    build/kernel/ttoskernel.o \
    build/kernel/isr.o \
    build/kernel/user_ring.o \
    -o build/kernel/kernel.elf

echo "[6/8] Creating kernel binary..."
objcopy -O binary \
    build/kernel/kernel.elf \
    build/kernel.bin

echo "[7/8] Creating disk image..."

dd if=/dev/zero \
   of=build/os.img \
   bs=512 \
   count=100 \
   status=none

dd if=build/boot.bin \
   of=build/os.img \
   bs=512 \
   seek=0 \
   conv=notrunc \
   status=none

dd if=build/kernel.bin \
   of=build/os.img \
   bs=512 \
   seek=1 \
   conv=notrunc \
   status=none

echo "[8/8] Appending userspace image..."

dd if=build/programs.img \
   of=build/os.img \
   bs=512 \
   seek=32 \ # place at the 32nd sector
   conv=notrunc \
   status=none

echo "Built build/os.img"
