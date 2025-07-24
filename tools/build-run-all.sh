#!/usr/bin/env bash

# Combines all the other build scripts for easier use.

# Get the absolute path to this script. 
export SCRIPT_LOCATION=$(cd -- $(dirname -- "${BASH_SOURCE[0]}"); pwd)

cd $SCRIPT_LOCATION

export COPY_HEADERS="n"
read -a COPY_HEADERS -p "Copy headers to sysroot? (y/n)"
if [ "$COPY_HEADERS" == "y" ]; then
    ./headers.sh
fi

./build-libc.sh
export RET_VAL=$?
if [ $RET_VAL == 0 ]; then
    echo libc was rebuilt, cleaning kernel so that it will be rebuild
    cd $SCRIPT_LOCATION/../kernel
    make clean
    cd $SCRIPT_LOCATION
# Abort because building libc failed.
elif [ $RET_VAL == 1 ]; then
    exit 1
fi

./build-kernel.sh
# Abort because building the kernel failed.
if [ $? != 0 ]; then
    exit 1
fi

./iso.sh
./run.sh