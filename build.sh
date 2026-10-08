#!/bin/bash

set -e

mkdir -p build

echo "[1/8] Assembling bootloader..."
nasm -f bin src/boot.asm -o build/boot.bin

echo "[2/8] Compiling kernel..."
g++ -m32 \
    -ffreestanding \
    -fno-pie \
    -fno-pic \
    -fno-exceptions \
    -fno-rtti \
    -fno-stack-protector \
    -c src/ttoskernel.cpp \
    -o build/ttoskernel.o

echo "[3/8] Assembling ISR..."
nasm -f elf32 src/isr.asm \
    -o build/isr.o

echo "[4/8] Assembling Ring 3 code..."
nasm -f elf32 src/user_ring.asm \
    -o build/user_ring.o

echo "[5/8] Compiling program..."
g++ -m32 \
    -ffreestanding \
    -fno-pie \
    -fno-pic \
    -fno-exceptions \
    -fno-rtti \
    -fno-stack-protector \
    -c src/main.cpp \
    -o build/main.o

echo "[6/8] Linking..."
ld -m elf_i386 \
    -T src/linker.ld \
    build/ttoskernel.o \
    build/isr.o \
    build/user_ring.o \
    build/main.o \
    -o build/kernel.elf

echo "[7/8] Creating kernel binary..."
objcopy -O binary \
    build/kernel.elf \
    build/kernel.bin

echo "[8/8] Creating disk image..."

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

echo "Built build/os.img"
