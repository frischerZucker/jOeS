#!/usr/bin/env bash

# Change to your toolchains location.
export PATH="$HOME/opt/cross/bin:$PATH"

# Get the absolute path to this script. 
export SCRIPT_LOCATION=$(cd -- $(dirname -- "${BASH_SOURCE[0]}"); pwd)
# Get the projects root directory. -> parent directory of where this script is located
export PROJECT_ROOT_DIR="$SCRIPT_LOCATION/.."

cd $PROJECT_ROOT_DIR

cd kernel
make TOOLCHAIN_PREFIX=x86_64-elf-
if [ $? != 0 ]; then
    echo "ERROR: Building the kernel failed!"
    exit
fi
cd $PROJECT_ROOT_DIR

make iso
if [ $? != 0 ]; then
    echo "ERROR: Creating an .iso failed!"
    exit
fi

make run
if [ $? != 0 ]; then
    echo "ERROR: Running the .iso using QEMU failed!"
    exit
fi