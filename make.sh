mkdir -p bin
mkdir -p obj

rm ./bin/kernel.elf
rm Ennea.iso

clang -m32 -c "src/boot.s" -o "obj/boot.o" -march=i386
clang -Wno-pragma-pack -m32 -Wpedantic -Wall -Wextra -march=i386 -I./src/inc -c -ffreestanding -msoft-float -fno-builtin "src/init.c" -o "obj/init.o"

ld -m elf_i386 -T "linker.ld" -o "bin/kernel.elf" -nostdlib "obj/boot.o" "obj/init.o"
if grub-file --is-x86-multiboot bin/kernel.elf; then
        echo multiboot confirmed
        mkdir -p isodir/boot/grub
        cp bin/kernel.elf isodir/boot/kernel.bin
        cp grub.cfg isodir/boot/grub/grub.cfg
        grub-mkrescue -o Ennea.iso isodir
        qemu-system-i386 -cdrom Ennea.iso -m 128 -monitor stdio -boot d
else
        echo the file is not multiboot
fi