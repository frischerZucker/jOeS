#!/usr/bin/env bash

# Sets some variables needed for the other scripts.

# Change to your toolchains location.
export PATH="$HOME/opt/cross/bin:$PATH"

# Get the absolute path to this script. 
export SCRIPT_LOCATION=$(cd -- $(dirname -- "${BASH_SOURCE[0]}"); pwd)
# Get the projects root directory. -> parent directory of where this script is located
export PROJECT_ROOT_DIR="$SCRIPT_LOCATION/.."

export SYSROOT="$PROJECT_ROOT_DIR/sysroot"

export PROJECTS="libc kernel"
export SYSTEM_HEADER_PROJECTS="libc kernel"

# Arguments to pass to QEMU.
export QEMU_ARGS="-cdrom image.iso \
                  -serial mon:stdio"