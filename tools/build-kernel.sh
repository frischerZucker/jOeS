#!/usr/bin/env bash

# Builds the kernel.

# Get the absolute path to this script. 
export SCRIPT_LOCATION=$(cd -- $(dirname -- "${BASH_SOURCE[0]}"); pwd)
# Get the projects root directory. -> parent directory of where this script is located
export PROJECT_ROOT_DIR="$SCRIPT_LOCATION/.."

cd $PROJECT_ROOT_DIR

. ./tools/config.sh

# Call the kernels GNUmakefile to build it.
cd kernel
make TOOLCHAIN_PREFIX=x86_64-elf-
if [ $? != 0 ]; then
    echo "ERROR: Building the kernel failed!"
    exit 1
fi