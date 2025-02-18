from elftools.elf.elffile import ELFFile
from unicorn.riscv_const import *
from unicorn.x86_const import *
from unicorn import *
from typing import Tuple
import argparse
import sys
import os

# os.environ['UNICORN_LOG_LEVEL'] = "0xFFFFFFFF"  # verbose - print anything
# os.environ['UNICORN_LOG_DETAIL_LEVEL'] = "1"  # full filename with line info


ARGS_OFFSET = 0x1000

num_reads = 0
num_writes = 0
num_execs = 0

inst_fetched_page = 0


def parse_cmdargs():
    parser = argparse.ArgumentParser(
        description="Emulate x64 code using Unicorn engine.")
    parser.add_argument('elf_file', type=str,
                        help='Path to the ELF file to emulate')
    parser.add_argument('-o', '--output', type=str, default=None,
                        help='Output file')
    parser.add_argument('-a', '--arch', type=str, choices=['x86_16b', 'x86', 'x64',
                        'rv32', 'rv64'], default=None, help='Architecture to emulate')
    parser.add_argument('-p', '--pagesize', type=int, default=4096,help='Page size')
    parser.add_argument('-f', '--fetch', action="store_true", default=False,
                        help='Enable instruction page fetch emulation')
    parser.add_argument('-g', '--argument', type=str, default=None,
                        help='Raw argument to pass to the program')
    return parser.parse_args()


args = parse_cmdargs()

if (args.pagesize & (args.pagesize - 1)) != 0:
    raise ValueError("Page size must be a power of 2")

PGSZ = (args.pagesize).bit_length() - 1


def out(s):
    fout.write(s + "\n")


def hook_mem_read(uc, access, address, size, value, user_data):
    global num_reads
    num_reads += 1
    # out("READ @ 0x%x [%u]" % (address, size))
    out("%u 1" % (address >> PGSZ))


def hook_mem_write(uc, access, address, size, value, user_data):
    global num_writes
    num_writes += 1
    # out("WRITE @ 0x%x [%u]" % (address, size))
    out("%u 2" % (address >> PGSZ))


def hook_code(uc, address, size, user_data):
    global num_execs
    num_execs += 1
    if (args.fetch):
        global inst_fetched_page
        if (inst_fetched_page != (address >> PGSZ)):
            inst_fetched_page = address >> PGSZ
            out("%u 4" % (address >> PGSZ))
    else:
        out("%u 4" % (address >> PGSZ))


def hook_mem_unmapped(uc, access, address, size, value, user_data):
    print("UNMAPPED MEM ACCESS at 0x%x [%u]" % (address, size))


if (args.elf_file == None):
    print("No ELF file provided")
    exit(1)

# Load ELF file
f = open(args.elf_file, 'rb')
elf = ELFFile(f)

if (args.arch == None):
    if elf.header.e_machine == 'EM_X86_64':
        args.arch = 'x64'
    elif elf.header.e_machine == 'EM_386':
        args.arch = 'x86'
    elif elf.header.e_machine == 'EM_RISCV':
        if elf.header.e_ident['EI_CLASS'] == 'ELFCLASS32':
            args.arch = 'rv32'
        else:
            args.arch = 'rv64'
    else:
        raise ValueError("Unknown architecture")

if (args.arch == 'x86_16b'):
    mu = Uc(UC_ARCH_X86, UC_MODE_16)
elif (args.arch == 'x86'):
    mu = Uc(UC_ARCH_X86, UC_MODE_32)
elif (args.arch == 'x64'):
    mu = Uc(UC_ARCH_X86, UC_MODE_64)
elif (args.arch == 'rv32'):
    mu = Uc(UC_ARCH_RISCV, UC_MODE_RISCV32)
elif (args.arch == 'rv64'):
    mu = Uc(UC_ARCH_RISCV, UC_MODE_RISCV64)
else:
    raise ValueError("Invalid architecture")

if (args.output != None):
    fout = open(args.output, "w")
else:
    fout = sys.stdout

print("Emulate code on %s" % (args.arch))


# Load all segments
entry_point = elf.header.e_entry
print("Entry point at 0x%x" % entry_point)
for segment in elf.iter_segments():
    if segment['p_type'] == 'PT_LOAD':
        address = segment['p_vaddr']
        size = segment['p_memsz']
        size = (size + 0xFFF) & ~0xFFF
        data = segment.data()
        pf = segment['p_flags']
        perm = 0
        perm |= UC_PROT_READ if pf & 0x4 else 0
        perm |= UC_PROT_WRITE if pf & 0x2 else 0
        perm |= UC_PROT_EXEC if pf & 0x1 else 0
        mu.mem_map(address, size, perm)
        print("Loading 0x%x bytes to 0x%x with perm 0%o" %
              (size, address, perm))
        mu.mem_write(address, data)

# Inject arguments
if (args.argument != None):
    try:
        mu.mem_map(ARGS_OFFSET, 0x1000, UC_PROT_READ | UC_PROT_WRITE)
    except UcError as e:
        pass
    print("Injecting argument into 0x%x: %s" % (ARGS_OFFSET, args.argument))
    arg = args.argument.encode('utf-8')
    mu.mem_write(ARGS_OFFSET, arg)

# print([_ for _ in mu.mem_regions()])

# initialize machine registers and stack
stack_section = elf.get_section_by_name('.stack')
if stack_section == None:
    stack_end = 0
else:
    stack_addr = stack_section['sh_addr']
    stack_size = stack_section['sh_size']
    stack_end = stack_addr + stack_size
print("Stack at 0x%x" % stack_end)

code_section = elf.get_section_by_name('.text')
code_begin = code_section['sh_addr']
code_size = code_section['sh_size']
code_end = code_begin + code_size

if (mu._arch == UC_ARCH_X86):
    if (mu._mode == UC_MODE_16):
        mu.reg_write(UC_X86_REG_SP, stack_end - 16)
        xlen = 2
    elif (mu._mode == UC_MODE_32):
        mu.reg_write(UC_X86_REG_ESP, stack_end - 16)
        xlen = 4
    else:
        mu.reg_write(UC_X86_REG_RSP, stack_end - 16)
        xlen = 8
    # We don't know if the funciton saves RBP or not, so we'd save 2 ret addresses off the stack
    retaddr = (code_end).to_bytes(xlen, byteorder='little')
    for i in range(16 // xlen):
        mu.mem_write(stack_end - 16 + i * xlen, retaddr)
elif (mu._arch == UC_ARCH_RISCV):
    mu.reg_write(UC_RISCV_REG_SP, stack_end - 16)
    mu.reg_write(UC_RISCV_REG_RA, code_end)


mu.hook_add(UC_HOOK_CODE, hook_code)
mu.hook_add(UC_HOOK_MEM_READ, hook_mem_read)
mu.hook_add(UC_HOOK_MEM_WRITE, hook_mem_write)
mu.hook_add(UC_HOOK_MEM_READ_UNMAPPED |
            UC_HOOK_MEM_WRITE_UNMAPPED |
            UC_HOOK_MEM_FETCH_UNMAPPED, hook_mem_unmapped)

print("Starting emulation at 0x%x, end = 0x%x" % (entry_point, code_end))
mu.emu_start(entry_point, code_end)

print("Function returned with ", end='')
if (mu._arch == UC_ARCH_X86):
    if (mu._mode == UC_MODE_16):
        print("AX = 0x%x" % mu.reg_read(UC_X86_REG_AX))
    elif (mu._mode == UC_MODE_32):
        print("EAX = 0x%x" % mu.reg_read(UC_X86_REG_EAX))
    else:
        print("RAX = 0x%x" % mu.reg_read(UC_X86_REG_RAX))
elif (mu._arch == UC_ARCH_RISCV):
    print("A0 = 0x%x, A1 = 0x%x" %
          (mu.reg_read(UC_RISCV_REG_A0), mu.reg_read(UC_RISCV_REG_A1)))

print("Done. %u instructions retired." % num_execs)
print("Reads: %u, Writes: %u" % (num_reads, num_writes))
