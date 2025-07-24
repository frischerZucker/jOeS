#!/usr/bin/env bash

# Builds libc and copies it into sysroot.

# Get the absolute path to this script. 
export SCRIPT_LOCATION=$(cd -- $(dirname -- "${BASH_SOURCE[0]}"); pwd)
# Get the projects root directory. -> parent directory of where this script is located
export PROJECT_ROOT_DIR="$SCRIPT_LOCATION/.."

cd $PROJECT_ROOT_DIR

. ./tools/config.sh

# Call libc's GNUmakefile to build it.
cd libc

# Check if libc needs to be rebuild.
make -q TOOLCHAIN_PREFIX=x86_64-elf-

if [ $? == 0 ]; then
    echo libc is already up to date.
    exit 2
else

    make TOOLCHAIN_PREFIX=x86_64-elf-

    if [ $? != 0 ]; then
        echo "ERROR: Building libc failed!"
        exit 1
    fi

    # Copies libc.a into the sysroot.
    mkdir --parent --verbose $PROJECT_ROOT_DIR/sysroot/usr/lib
    cp *.a $PROJECT_ROOT_DIR/sysroot/usr/lib/

    exit 0
fi