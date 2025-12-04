"""
Retrieves information from a page table entry.
"""
from typing_extensions import Literal

import sys

def print_pt_info(pt_entry: int, entry_type: Literal["pml4", "pdpr", "pd", "pt"]) -> None:
    print(f"Raw entry:\t\t0x{pt_entry:016x}") 
    print(f"Present:\t\t{"1\t\t\tpresent" if pt_entry & (1 << 0) else "0\t\t\tnot present"}")
    print(f"Read/Write:\t\t{"1\t\t\tread/write" if pt_entry & (1 << 1) else "0\t\t\tread only"}") 
    print(f"User/Supervisor:\t{"1\t\t\tuser access allowed" if pt_entry & (1 << 2) else "0\t\t\tsupervisor access only"}")
    print(f"Wright-Through:\t\t{"1\t\t\twrite-through caching enabled" if pt_entry & (1 << 3) else "0\t\t\twrite-back enabled"}")
    print(f"Cache Disable:\t\t{"1\t\t\tcache disabled" if pt_entry & (1 << 4) else "0\t\t\tcache enabled"}")
    print(f"Accessed:\t\t{"1\t\t\taccessed" if pt_entry & (1 << 5) else "0\t\t\tnot accesed"}")

    if entry_type == "pt":
        print(f"Dirty:\t\t\t{"1\t\t\tpage was written to" if pt_entry & (1 << 6) else "0\t\t\tpage was not written to"}")
        print(f"Page Size:\t\t{"1" if pt_entry & (1 << 7) else "0"}")
        print(f"Global:\t\t\t{"1\t\t\tglobal" if pt_entry & (1 << 8) else "0\t\t\tnot global"}")
    else:
        print(f"Reserved:\t\t{(pt_entry >> 6) & 0x7}\t\t\t{"not ok" if (pt_entry >> 6) & 0x7 else "ok"}")

    print(f"Available:\t\t{"1" if pt_entry & (0b111 << 9) else "0"}")
    print(f"{"Page" if entry_type == "pt" else "Table"} Base Address:\t0x{((pt_entry >> 12) & 0x7ffffff):07x}")
    print(f"Reserved:\t\t{(pt_entry >> 40) & 0x7ff}\t\t\t{"not ok" if (pt_entry >> 40) & 0x7ff else "ok"}")
    print(f"Available:\t\t{(pt_entry >> 52) & 0x3f}")
    print(f"Protection Key:\t\t{(pt_entry >> 59) & 0xf}")
    print(f"Execute Disable:\t{"1\t\t\tcode exectution disabled" if pt_entry & (1 << 63) else "0\t\t\tcode execution enabled"}")

    if entry_type != "pt":
        assert (pt_entry >> 6) & 0x7 == 0, f"Reserved field [6:8] should be 0. (it is {(pt_entry >> 6) & 0x7})"
    assert (pt_entry >> 40) & 0x7ff == 0, f"Reserved field [40:51] should be 0. (it is {(pt_entry >> 40) & 0x7ff})"
    

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"Missing arguments! Use '{sys.argv[0]} --help' to get a list of possible arguments.")
        sys.exit(-1)
    else:
        pt_entry: int = -1
        pt_entry_type: Literal["pml4", "pdpr", "pd", "pt"] = "pt"
        pt_entry_base: int = 16

        arg: str
        state: Literal["normal", "type", "base"] = "normal"
        for arg in sys.argv[1:]:
            match state:
                case "normal":
                    if arg.startswith("--"):
                        if arg[2:] == "help":
                            print("This script shows the fields of a page table entry.")
                            print(f"{sys.argv[0]} [--help] [--base BASE] [--type TYPE] PAGE_TABLE_ENTRY")
                            print("--help - print this text")
                            print("--base - base used for the page table entry, e.g. 10=dec, 16=hex, default: hex")
                            print("--type - type of table the entry is from (pml4, pdpr, pd, pt)")
                            print("PAGE_TABLE_ENTRY - table entry as integer")
                            sys.exit(0)
                        state = arg[2:]
                    else:
                        pt_entry = int(arg, base=pt_entry_base)
                case "type":
                    pt_entry_type = arg
                    state = "normal"
                case "base":
                    pt_entry_base = int(arg)
                    state = "normal" 
                case _:
                    print(f"Unkown argument: {state}")
                    sys.exit(-1)              

        assert pt_entry >= 0, f"No valid entry was passed. (entry={pt_entry})"

        print_pt_info(pt_entry, pt_entry_type)