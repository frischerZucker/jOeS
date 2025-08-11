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

def help() -> None:
    print(
            "This script helps to automate the creation of IDT-Entries.\n" \
            "Possible arguments are:\n" \
            "\t--entries (Prints calls to idt_create_entry() for all IDT-Entries.)\n" \
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
        case "--help":
            help()
        case _:
            print(f"Unknown argument: {sys.argv[1]}")