# jOeS

**jOeS** is a hobby project to create an operating system for `x86_64` using C and the [Limine Bootloader](https://github.com/limine-bootloader/limine).

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

## Project Goals

~~My current goal is to implement a basic terminal that outputs text to the screen using a framebuffer, acting as a minimal "Hello World" demonstration.~~ I got a framebuffer up and running with a simple terminal and a (not quite finished) printf() function! My next goal is to get interrupts working. Let's see how that goes.. :D

## Project Structure
```
/
├── kernel/                    
|         ├── include/        # kernel headers
|         ├── linker_scripts/ # kernel linker scripts
|         ├── src/            # kernel source files
|         └── GNUmakefile     # kernel makefile
├── libc/                    
|       ├── include/          # libc headers
|       ├── stdio/            # stdio.h source files
|       ├── string/           # string.h source files
|       └── GNUmakefile       # libc makefile
├── tools/                    # build scripts & additional tools
├── README.md                 # you are here lol
└── limine.conf               # limine config file
```

## Requirements

To build and run **jOeS** you need:
- a `x86_64-elf-gcc` cross compiler toolchain as described [here](https://wiki.osdev.org/GCC_Cross-Compiler)
- GNU `make`
- NASM
- QEMU

For some of the additional tools you also need:
- Python

## Building and Running

To build libc run the following scripts:
``` bash
./headers.sh
./build-libc.sh
```

To build the kernel, create an iso and run it use:
``` bash
./build-kernel.sh
./iso.sh
./run.sh
```

Or simply use the combined script to do everything at once:
``` bash
./tools/build-run-all.sh
```

## Contributing

For now it's just me, myself and I.

## Rote Beete?
- Rote Beete = 0