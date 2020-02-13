echo 'Make system.img'
dd if=/dev/zero of=system.img bs=512 count=2880

echo 'Compile bootloader.asm'
nasm bootloader.asm -o bootloader

echo 'Copy bootloader to system.img'
dd if=bootloader of=system.img bs=512 count=1 conv=notrunc

echo 'Copy map.img to system.img'
dd if=map.img of=system.img bs=512 count=1 seek=1 conv=notrunc

echo 'Copy dir.img to system.img'
dd if=dir.img of=system.img bs=512 count=1 seek=2 conv=notrunc

echo 'Compile kernel.c'
bcc -ansi -c -o kernel.o kernel.c

echo 'Compile kernel.asm'
nasm -f as86 kernel.asm -o kernel_asm.o

echo 'Link kernel.o dan kernel_asm.o'
ld86 -o kernel -d kernel.o kernel_asm.o

echo 'Copy kernel ke system.img'
dd if=kernel of=system.img bs=512 conv=notrunc seek=3

echo 'Start the emulator'
bochs -f if2230.config
