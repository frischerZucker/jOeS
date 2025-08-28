#!/usr/bin/env bash

# Checks if all dependencies are installed.
# Prompts the user to install them if not.
# Only works on distros using apt as package manager.

# Checks if a package is installed.
#
# @returns '0' if the package is installed, otherwise '1'.
is_pkg_installed() {
    dpkg-query -s $1 2>/dev/null | grep -q "install ok installed"
    echo $?
}

. ./tools/config.sh

RED='\033[0;31m'
GREEN='\033[0;32m'
WHITE='\033[1;37m'

DEPENDENCIES="make nasm qemu-system-x86 xorriso"
NUM_MISSING_DEPENDENCIES=0

# Check if apt is used as package manager.
command -v apt > /dev/null
if [[ $? != 0 ]]; then
    echo "This script only works with apt and it seems like you ain't using it :("
    exit 1
fi

# Check dependencies that are managed by the package manager.
echo "The following dependencies are required:"
for dep in ${DEPENDENCIES[@]}
do
    INSTALLED=$(is_pkg_installed $dep)

    if [[ $INSTALLED = 0 ]]; then # Package is installed.
        echo -e "- ${dep}: [${GREEN}installed${WHITE}]"
    else
        echo -e "- ${dep}: [${RED}not installed${WHITE}]"
        NUM_MISSING_DEPENDENCIES=$((NUM_MISSING_DEPENDENCIES + 1))
    fi
done

# Check if there are missing dependencies. If so, ask to install them.
if [[ $NUM_MISSING_DEPENDENCIES > 0 ]]; then
    AUTO_INSTALL="n"
    read -a AUTO_INSTALL -p "Install missing dependencies? (y/n) "

    # Automatically install missing packages.
    if [ "$AUTO_INSTALL" = "y" ]; then
        sudo apt update

        for dep in ${DEPENDENCIES[@]}
        do
            # Install the package if it is not installed
            if [[ $(is_pkg_installed $dep) == 1 ]]; then
                echo "Installing ${dep}.."
                sudo apt install $dep
            fi
        done
    fi
fi

echo "All dependencies are installed."

# Check if a x86_64-elf cross compiler exists in ~/opt/cross/.
which "${TOOLCHAIN_PREFIX}gcc" > /dev/null
if [[ $? = 1 ]]; then
    echo "It seems like there is no x86_64-elf-toolchain in ~/opt/cross/."
    echo "If you have none, you need to build one."
fi