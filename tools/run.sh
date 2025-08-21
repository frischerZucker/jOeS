#!/usr/bin/env bash

# Get the absolute path to this script. 
export SCRIPT_LOCATION=$(cd -- $(dirname -- "${BASH_SOURCE[0]}"); pwd)
# Get the projects root directory. -> parent directory of where this script is located
export PROJECT_ROOT_DIR="$SCRIPT_LOCATION/.."

cd $PROJECT_ROOT_DIR

. ./tools/config.sh

qemu-system-x86_64 $QEMU_ARGS