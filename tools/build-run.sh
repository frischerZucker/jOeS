#!/usr/bin/env bash

# Get the absolute path to this script. 
export SCRIPT_LOCATION=$(cd -- $(dirname -- "${BASH_SOURCE[0]}"); pwd)

cd $SCRIPT_LOCATION

./build.sh
./iso.sh
./run.sh