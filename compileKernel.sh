echo 'Compile kernel.c'
bcc -ansi -c -o kernel.o kernel.c

echo 'Compile kernel.asm'
nasm -f as86 kernel.asm -o kernel_asm.o

echo 'Link kernel.o and kernel_asm.o'
ld86 -o kernel -d kernel.o kernel_asm.o

echo 'Copy kernel to system.img'
dd if=kernel of=system.img bs=512 conv=notrunc seek=3

echo 'Start the emulator'
bochs -f if2230.config
