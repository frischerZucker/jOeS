#!/usr/bin/env bash

export ISO_ROOT=iso_root    # root directory for the iso creation
export ISO_FILE=image.iso   # name of the .iso file to create

# Get the absolute path to this script. 
export SCRIPT_LOCATION=$(cd -- $(dirname -- "${BASH_SOURCE[0]}"); pwd)
# Get the projects root directory. -> parent directory of where this script is located
export PROJECT_ROOT_DIR="$SCRIPT_LOCATION/.."

cd $PROJECT_ROOT_DIR

# Check if Limine repository is present, clone it if not.
cd limine
if [ $? != 0 ]; then
    echo "Limine repository is missing. Cloning it from git..."

    git clone https://github.com/limine-bootloader/limine.git --branch=v9.x-binary --depth=1
    cd limine
fi

echo "Building Limine..."
make

cd $PROJECT_ROOT_DIR

# Create directories needed for our iso.
echo "Root directory for iso creation: $ISO_ROOT."
echo "Setting up $ISO_ROOT..." 
mkdir --parents --verbose $ISO_ROOT
mkdir --parents --verbose $ISO_ROOT/EFI/BOOT
mkdir --parents --verbose $ISO_ROOT/boot

# Copying limine into the iso directory.
echo "Copying files to $ISO_ROOT..."
cp --verbose limine/limine-uefi-cd.bin $ISO_ROOT
cp --verbose limine/limine-bios-cd.bin $ISO_ROOT
cp --verbose limine/limine-bios.sys $ISO_ROOT
cp --verbose limine.conf $ISO_ROOT
cp --verbose limine/BOOTX64.EFI $ISO_ROOT/EFI/BOOT
# Copying the kernel in the iso directory.
cp --verbose kernel/bin/* $ISO_ROOT/boot

# Use xorriso to create the iso.
# Command is from https://github.com/limine-bootloader/limine/blob/v9.x/USAGE.md.
echo "Creating the iso..."
xorriso -as mkisofs -R -r -J -b limine-bios-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
        -apm-block-size 2048 --efi-boot limine-uefi-cd.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        $ISO_ROOT -o $ISO_FILE
# Exit if iso creation failed.
if [ $? != 0 ]; then
    echo "ERROR: ISO creation failed!"
    exit 1
fi

# Install Limine on the iso.
echo "Installing Limine..."
limine/limine bios-install $ISO_FILE

echo "ISO creation finished."