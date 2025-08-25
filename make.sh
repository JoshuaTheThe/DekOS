mkdir -p bin
mkdir -p obj

rm ./bin/kernel.elf
rm Ennea.iso

clang -m32 -c "src/boot.s" -o "obj/boot.o" -march=i386
clang -Wno-pragma-pack -m32 -Wpedantic -Wall -Wextra -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin "src/init.c" -o "obj/init.o"
clang -Wno-pragma-pack -m32 -Wpedantic -Wall -Wextra -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin "src/fonts.c" -o "obj/fonts.o"
clang -Wno-pragma-pack -m32 -Wpedantic -Wall -Wextra -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin "src/text.c" -o "obj/text.o"
clang -Wno-pragma-pack -m32 -Wpedantic -Wall -Wextra -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin "src/ps2.c" -o "obj/ps2.o"
clang -Wno-pragma-pack -m32 -Wpedantic -Wall -Wextra -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin "src/idt.c" -o "obj/idt.o"
clang -Wno-pragma-pack -m32 -Wpedantic -Wall -Wextra -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin "src/io.c" -o "obj/io.o"
clang -Wno-pragma-pack -m32 -Wpedantic -Wall -Wextra -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin "src/gdt.c" -o "obj/gdt.o"

ld -m elf_i386 -T "linker.ld" -o "bin/kernel.elf" -nostdlib "obj/boot.o" "obj/init.o" "obj/fonts.o" "obj/text.o" "obj/ps2.o" "obj/idt.o" "obj/io.o" "obj/gdt.o"
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
