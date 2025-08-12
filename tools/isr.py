"""
Generates C Code used for the creation of IDT-Entries.
"""

import sys

def extern_isrs() -> None:
    """
    Prints the extern declarations for the _isr* from isr.asm.
    """
    print("extern void ", end="")
    for i in range(0, 255):
        print(f"_isr0x{f"{i:02X}".lower()}(), ", end="")
    print("_isr255();") 

def idt_entries() -> None:
    """
    Prints the calls to idt_create_entry() for all 256 IDT-Entries.
    """
    for i in range(0, 256):
        print(f"idt_create_entry(&idt[{i}], _isr0x{f"{i:02X}".lower()}, 0x8, 0, IDT_ATTRIBUTES(0, IDT_GATE_TYPE_INT_GATE));")

def enum() -> None:
    """
    Prints interrupt names for the interrupt enum.
    """
    print("INT_DIVIDE_ERR,")
    print("INT_DEBUG_EXCEPTION,")
    print("INT_NMI_INT,")
    print("INT_BREAKPOINT,")
    print("INT_OVERFLOW,")
    print("INT_BOUND_RANGE_EXCEEDED,")
    print("INT_INVALID_OPCODE,")
    print("INT_NO_MATH_COPROCESSOR,")
    print("INT_DOUBLE_FAULT,")
    print("INT_COPROCESSOR_SEG_OVERRUN,")
    print("INT_INVALID_TSS,")
    print("INT_SEGMENT_NOT_PRESENT,")
    print("INT_STACK_SEGMENT_FAULT,")
    print("INT_GENERAL_PROTECTION_FAULT,")
    print("INT_PAGE_FAULT,")
    print("INT_RESERVED0,")
    print("INT_FPU_FLOATING_POINT_ERR,")
    print("INT_ALIGNMENT_CHECK,")
    print("INT_MACHINE_CHECK,")
    print("INT_SIMD_FLOATING_POINT_EXCEPTION,")
    print("INT_VIRTUALIZATION_EXCEPTION,")
    print("INT_CONTROL_PROTECTION_EXCEPTION,")
    for i in range(1, 11):
        print(f"INT_RESERVED{i},")
    for i in range(0, 255 - 31 - 1):
        print(f"INT_EXT_INT{i},")
    print(f"INT_EXT_INT224")

def help() -> None:
    print(
            "This script helps to automate the creation of IDT-Entries.\n" \
            "Possible arguments are:\n" \
            "\t--entries (Prints calls to idt_create_entry() for all IDT-Entries.)\n" \
            "\t--enum (Prints interrupt names for the interrupt enum.)" \
            "\t--externs (Prints the extern declarations of th ISRs from isr.asm.)\n"
    )

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"Missing arguments! Use '{sys.argv[0]} --help' to get a list of possible arguments.")
        exit(-1)

    match sys.argv[1]:
        case "--entries":
            idt_entries()
        case "--externs":
            extern_isrs()
        case "--enum":
            enum();
        case "--help":
            help()
        case _:
            print(f"Unknown argument: {sys.argv[1]}")