# jOeS

**jOeS** is a hobby project to create a meme operating system for `x86_64` using C and the [Limine Bootloader](https://github.com/limine-bootloader/limine).

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

My current goal is to implement a basic terminal that outputs text to the screen using a framebuffer, acting as a minimal "Hello World" demonstration.

## Project Structure
```
/
├── kernel/                    
|         ├── linker_scripts/  # linker scripts used for the kernel
|         ├── src/             # source code of the kernel
|         └── GNUmakefile      # makefile for building the kernel
├── tools/                     # additional tools for development
├── build.sh                   # script to easily build and run jOeS
├── GNUmakefile                # makefile for .iso-creation and running
└── README.md                  # you are here lol
```

## Requirements

To build and run **jOeS** you need:
- a cross-compiled `x86_64-elf-gcc` toolchain as described [here](https://wiki.osdev.org/GCC_Cross-Compiler)
- GNU `make`
- QEMU

For some of the additional tools you also need:
- Python

## Building and Running

Run the following commands from the project root directory:
``` bash
cd kernel
make
cd ..
make iso
make run
```
Or simply run the provided script:
``` bash
./build-run.sh
```

## Contributing

For now it's just me, myself and I.

## Rote Beete?
- Rote Beete = 0