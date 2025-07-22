#!/usr/bin/env bash

# Copies the kernels and libc's headers into sysroot.

# Get the absolute path to this script. 
export SCRIPT_LOCATION=$(cd -- $(dirname -- "${BASH_SOURCE[0]}"); pwd)
# Get the projects root directory. -> parent directory of where this script is located
export PROJECT_ROOT_DIR="$SCRIPT_LOCATION/.."

cd $PROJECT_ROOT_DIR

. ./tools/config.sh

mkdir --parent --verbose $SYSROOT/usr/include

for PROJECT in $SYSTEM_HEADER_PROJECTS; do
    (
        cd $PROJECT
        cp --verbose include/*.h $SYSROOT/usr/include/
    )
done