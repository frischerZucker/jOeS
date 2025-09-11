# jOeS
```
          
         /´´´´´\
        | () () |   <- wise, slightly tired eyes
        |   ^   |   
        | \___/ |   <- gentle smile
       /|-|-+-|-|\
      / | | A | | \  <- suit collar
     *  | | V | |  *
        |__\_/__| 
        /  | |  \    
       /   | |   \   <- formal politician's pants
      (____| |____)  

     "Come on, man!"
```

**jOeS** is a hobby project to create an operating system for `x86_64` using C and the [Limine Bootloader](https://github.com/limine-bootloader/limine).
My long-term goal is to create an OS that is able to run simple programms, preferrably on real hardware.
As of now, I managed to write a lil kernel that can handle interrupt, print to a framebuffer. 
It also includes some drivers for some (partially outdated :|) devices:
- 8259 PIC
- 8254 PIT
- I8042 PS/2 Controller
- PS/2 Keyboards
- Serial Controller

The next chapter on this journey will be memory management. I plan on implementing physical and virtual memory managers as my next step.
Let's see how this will turn out and how long it takes! :D 

## Project Structure
```
/
├── doku/                  
|       ├── html/                   # Project documentation
|       └── img/                    # Images used in the documentation
├── kernel/                    
|         ├── include/           
|         |          ├── cpu/       # Headers for CPU specific code
|         |          ├── drivers/   # Driver headers
|         |          └── *.h        # General headers
|         ├── linker_scripts/       # kernel linker scripts
|         ├── src/               
|         |      ├── cpu/           # Source files for CPU specific code
|         |      ├── drivers/       # Driver source files
|         |      └── *.c            # General source files
|         └── GNUmakefile           # kernel makefile
├── libc/                    
|       ├── include/                # libc headers
|       ├── stdio/                  # stdio.h source files
|       ├── string/                 # string.h source files
|       └── GNUmakefile             # libc makefile    
├── tools/                          # build scripts & additional tools
├── .gitignore             
├── Doxyfile                        # Settings for documentation
├── LICENSE                         # BSD 2-Clause license
├── limine.conf                     # limine config file
└── README.md                       # you are here lol
```

## Requirements

To build and run **jOeS** you need:
- a `x86_64-elf-gcc` cross compiler toolchain as described [here](https://wiki.osdev.org/GCC_Cross-Compiler)
- GNU `make`
- NASM
- `qemu-system-x86_64`

If you are using a Linux Distro that uses `apt` as package manager, you can run check for missing dependencies and install them by running:
``` bash
./tools/check-dependencies.sh
```

For some of the additional tools you also need:
- Python

## Building and Running

To build libc run the following scripts:
``` bash
./tools/headers.sh
./tools/build-libc.sh
```

To build the kernel, create an iso and run it use:
``` bash
./tools/build-kernel.sh
./tools/iso.sh
./tools/run.sh
```

Or simply use the combined script to do everything at once:
``` bash
./tools/build-run-all.sh
```

## Contributing

For now it's just me, myself and I.

## Rote Beete?
- Rote Beete = 0