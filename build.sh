#!/bin/bash

set -e

mkdir -p build

echo "[1/7] Assembling bootloader..."
nasm -f bin src/boot.asm -o build/boot.bin

echo "[2/7] Compiling kernel..."
g++ -m32 \
    -ffreestanding \
    -fno-pie \
    -fno-pic \
    -fno-exceptions \
    -fno-rtti \
    -fno-stack-protector \
    -c src/ttoskernel.cpp \
    -o build/ttoskernel.o

echo "[3/7] Assembling ISR..."
nasm -f elf32 src/isr.asm \
    -o build/isr.o

echo "[4/7] Compiling program..."
g++ -m32 \
    -ffreestanding \
    -fno-pie \
    -fno-pic \
    -fno-exceptions \
    -fno-rtti \
    -fno-stack-protector \
    -c src/main.cpp \
    -o build/main.o

echo "[5/7] Linking..."
ld -m elf_i386 \
    -T src/linker.ld \
    build/ttoskernel.o \
    build/isr.o \
    build/main.o \
    -o build/kernel.elf

echo "[6/7] Creating kernel binary..."
objcopy -O binary \
    build/kernel.elf \
    build/kernel.bin

echo "[7/7] Creating disk image..."

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
