# TungTung OS
A 32-Bit, multi-ring, operating system, written from scratch in just assembly and C++, capable
of running on real hardware, using a custom Bootloader targetting legacy BIOS systems.

![tung tung calculator demo](images/demo.png)

## Technical Details
 - Premtive Multitasking Support
 - Seperately compiled userspace
 - Custom Bootloader written in x86 assembly
 - Keyboard Driver
 - VGA Teletype Driver
 - Support for SSE Instructions
 - Paging
 - Ring 3 Support with custom syscalls
 - Kernel Panics:

![Division by zero kernel panic](images/panic.png)

## Sign me up!!
Install today! Go to the releases tab and get your .img today, launch with qemu, or run on real
hardware if you dare.

Run on Qemu:
```
qemu-system-i386 -drive format=raw,file=<FILE NAME>
```  

Flash to a real iso (at your own risk!)
```
sudo dd if=<FILE NAME> of=/dev/sdX bs=4M status=progress conv=fsync
```

## Build
Ensure `python3`, `nasm`, `g++`, and any linker that supports intel i386 is installed. Run:

```sh
python3 build.py
chmod +x build.sh
./build.sh
```

## Write a program yourself!
Feel free to modify any program in `src/user` yourself! These use a custom "standard" library found in `src/user/stdlib.h`
Reminder that you must run `build.py` for any of your userspace changes to compile.

## Organization
Here's a small guide to the OS to get you started:

```
$ tree src
src
├── kernel
│   ├── boot.asm        # Custom bootloader. Targets x86 Legacy Boot(BIOS) systems
│   ├── isr.asm         # ISR Handlers. Called by the CPU on interrupts
│   ├── ttoskernel.cpp  # The entirety of the TTOS Kernel
│   ├── ttoskernel.h    # Headers
│   └── user_ring.asm   # Starting point for userspace programs
└── user
    ├── program_a.cpp   # Program A
    ├── program_b.cpp   # Program B
    ├── program_c.cpp   # Program C
    ├── program_d.cpp   # Program D
    ├── stdlib.cpp      # Standard Library Implementation
    └── stdlib.h        # Standard Library Specification Header

3 directories, 11 files
```

If you want to contribute for any reason, feel free to send in a PR.

## For the future
Ideas:
- Networking Drivers
- Framebuffer screen driver
    - Desktop Environment
    - Inter-task communication via shared dynamically allocated buffers
- Shell
- libc compatibility
    - c++ stl???
- Sound Driver
