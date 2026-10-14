from pathlib import Path
import subprocess
import struct

ROOT = Path(__file__).resolve().parent
BUILD = ROOT / "build"
USER_BUILD = BUILD / "user"
GENERATED = BUILD / "generated"

USER_SRC = ROOT / "src" / "user"
USER_LD = ROOT / "linker" / "user.ld"

STDLIB_SRC = USER_SRC / "stdlib.cpp"

IMAGE = BUILD / "programs.img"

# programs are aligned to 4096 so that the kernel can just map a virtual page to it
PAGE_SIZE = 4096
SECTOR_SIZE = 512
USER_BASE = 0x00100000

PROGRAMS = [
    ("program_a", USER_SRC / "program_a.cpp"),
    ("program_b", USER_SRC / "program_b.cpp"),
    ("program_c", USER_SRC / "program_c.cpp"),
    ("program_d", USER_SRC / "program_d.cpp"),
]

# todo: make this clear in readme
CXX = "g++"
LD = "ld"
OBJCOPY = "objcopy"
NM = "nm"

# same as build.sh
CXXFLAGS = [
    "-m32",
    "-std=c++17",
    "-ffreestanding",
    "-fno-pie",
    "-fno-pic",
    "-fno-exceptions",
    "-fno-rtti",
    "-fno-stack-protector",
    "-fno-use-cxa-atexit",
    "-fno-threadsafe-statics",
    "-fno-asynchronous-unwind-tables",
    "-fno-unwind-tables",
    "-nostdlib",
    "-Wall",
    "-Wextra",
    "-O2",
]

# nice shell wrapper
def run(*args):
    print("+", " ".join(map(str, args)))
    subprocess.run(args, check=True)

def get_symbols_from_elf(elf):
    o = subprocess.check_output([NM, "-n", str(elf)], text=True)

    symbols = {}

    for l in o.splitlines():
        f = l.split()
        if len(f) == 3:
            address, _kind, name = f
            symbols[name] = int(address, 16)

    return symbols

def build_program(name, source):
    elf = USER_BUILD / f"{name}.elf"
    binary = USER_BUILD / f"{name}.bin"

    # compile userspace programs
    run(CXX, *CXXFLAGS, "-c", str(source), "-o", str(USER_BUILD/f"{name}.o"))

    run(CXX, *CXXFLAGS, "-c", str(STDLIB_SRC), "-o", str(USER_BUILD/"stdlib.o"))

    # link userspace programs
    run(LD,"-m", "elf_i386","-T", str(USER_LD),"-o", str(elf), str(USER_BUILD / f"{name}.o"), str(USER_BUILD / "stdlib.o"))

    # flatten to .bin
    run(OBJCOPY, "-O", "binary", str(elf), str(binary))

    symbols = get_symbols_from_elf(elf)

    entry = symbols["_start"]
    file_end = symbols["__program_file_end"]
    mem_end = symbols["__program_mem_end"]

    file_size = binary.stat().st_size
    expected_file_size = file_end - USER_BASE

    if file_size != expected_file_size:
        raise Exception("linker did not procure a binary of the expected size")
    if entry < USER_BASE or entry >= file_end:
        raise RuntimeError(f"{name}: _start is outside file backed data")

    # object
    return {
        "name": name,
        "binary": binary,
        "entry": entry,
        "file_size": file_size,
        "memory_size": mem_end - USER_BASE,
    }

def main():
    # create folders for first time compilers
    USER_BUILD.mkdir(parents=True, exist_ok=True)
    GENERATED.mkdir(parents=True, exist_ok=True)

    built = [ build_program(name, source) for name,source in PROGRAMS ]
    offset = 0
    meta = []
    with (IMAGE.open("wb") as image):
        for program in built:
            #ensure 4096 alignment
            assert offset % PAGE_SIZE == 0

            image.write(program["binary"].read_bytes())
            padded_size = ((len(program["binary"].read_bytes()) + PAGE_SIZE - 1) // PAGE_SIZE) * PAGE_SIZE
            image.write(b"\0" * (padded_size - len(program["binary"].read_bytes())))
            meta.append({
                **program,
                "offset": offset,
                "lba": offset//SECTOR_SIZE,
                "sector_count": padded_size // SECTOR_SIZE,
                "padded_size": padded_size
            })
            offset += padded_size

    for p in meta:
        print(
            f'{p["name"]}: '
            f'offset=0x{p["offset"]:X}, '
            f'LBA+{p["lba"]}, '
            f'sectors={p["sector_count"]}, '
            f'file={p["file_size"]}, '
        )

    header_path = GENERATED / "programs.h"

    with header_path.open("w") as header:
        header.write("""#pragma once
#define uint32_t unsigned int
namespace ttos {

struct ProgramInfo {
    const char* name;
    uint32_t offset_bytes;
    uint32_t start_lba; // note: relative lba
    uint32_t sector_count;
    uint32_t file_size;
    uint32_t padded_size;
    uint32_t memory_size;
    uint32_t entry_address;
};

""")

        header.write(
            f"inline constexpr uint32_t PROGRAM_COUNT = {len(meta)};\n"
        )
        header.write(
            "inline constexpr ProgramInfo programs[PROGRAM_COUNT] = {\n"
        )

        for p in meta:
            header.write(
                f'    {{"{p["name"]}", '
                f'{p["offset"]}u, '
                f'{p["lba"]}u, '
                f'{p["sector_count"]}u, '
                f'{p["file_size"]}u, '
                f'{p["padded_size"]}u, '
                f'{p["memory_size"]}u, '
                f'0x{p["entry"]:08X}u}},\n'
            )

        header.write("};\n\n} // namespace ttos\n")

    print(f"Generated {header_path}")
    
main()
