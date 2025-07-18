# Change to your toolchains location.
export PATH="$HOME/opt/cross/bin:$PATH"

cd kernel
make TOOLCHAIN_PREFIX=x86_64-elf-
if [ $? != 0 ]; then
    echo "ERROR: Building the kernel failed!"
    exit
fi
cd ..

make iso
if [ $? != 0 ]; then
    echo "ERROR: Creating an .iso failed!"
    exit
fi

make run
if [ $? != 0 ]; then
    echo "ERROR: Running the .iso using QEMU failed!"
    exit
fi