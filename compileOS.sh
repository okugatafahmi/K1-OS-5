echo 'Make system.img'
dd if=/dev/zero of=system.img bs=512 count=2880

echo 'Compile bootloader.asm'
nasm bootloader.asm -o bootloader

echo 'Copy bootloader to system.img'
dd if=bootloader of=system.img bs=512 count=1 conv=notrunc

echo 'Copy map.img to system.img'
dd if=map.img of=system.img bs=512 count=1 seek=256 conv=notrunc

echo 'Copy files.img to system.img'
dd if=files.img of=system.img bs=512 count=2 seek=257 conv=notrunc

echo 'Copy sectors.img to system.img'
dd if=sectors.img of=system.img bs=512 count=1 seek=259 conv=notrunc

echo 'Compile kernel.asm'
nasm -f as86 kernel.asm -o kernel_asm.o

echo 'Compile lib.asm'
nasm -f as86 lib.asm -o lib_asm.o

echo 'Compile kernel.c'
bcc -ansi -c -o kernel.o kernel.c || exit 1

echo 'Link kernel.o and kernel_asm.o'
ld86 -o kernel -d kernel.o kernel_asm.o

echo 'Copy kernel to system.img'
dd if=kernel of=system.img bs=512 conv=notrunc seek=1

echo "Compile loadFile.c"
gcc loadFile.c -o loadFile || exit 1

echo 'Run cnl.sh'
./cnl.sh shell.c mikro.c || exit 1

echo 'Load key.txt to system.img'
./loadFile key.txt

echo 'Start the emulator'
bochs -f if2230.config
