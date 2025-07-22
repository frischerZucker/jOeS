#!/usr/bin/env bash

# Combines all the other build scripts for easier use.

# Get the absolute path to this script. 
export SCRIPT_LOCATION=$(cd -- $(dirname -- "${BASH_SOURCE[0]}"); pwd)

cd $SCRIPT_LOCATION

./headers.sh
./build-libc.sh
./build-kernel.sh
./iso.sh
./run.sh