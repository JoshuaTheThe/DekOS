mkdir -p bin 
mkdir -p obj 
 
rm ./bin/kernel.elf 
rm dekos.iso 
rm ./obj/* 
 
clang -m32 -c "src/boot.s" -o "obj/boot.o" -march=i386 
clang -Wno-pragma-pack -m32 -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin "src/main.c" -o "obj/main.o" 
clang -Wno-pragma-pack -m32 -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin "src/fonts.c" -o "obj/fonts.o" 
clang -Wno-pragma-pack -m32 -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin "src/text.c" -o "obj/text.o" 
clang -Wno-pragma-pack -m32 -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin "src/input.c" -o "obj/input.o" 
clang -Wno-pragma-pack -m32 -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin "src/idt.c" -o "obj/idt.o" 
clang -Wno-pragma-pack -m32 -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin "src/io.c" -o "obj/io.o" 
clang -Wno-pragma-pack -m32 -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin "src/gdt.c" -o "obj/gdt.o" 
clang -Wno-pragma-pack -m32 -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin "src/math.c" -o "obj/math.o" 
clang -Wno-pragma-pack -m32 -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin "src/disk.c" -o "obj/disk.o" 
clang -Wno-pragma-pack -m32 -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin "src/iso9660.c" -o "obj/iso9660.o" 
clang -Wno-pragma-pack -m32 -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin "src/memory.c" -o "obj/memory.o" 
 
ld -m elf_i386 -T "linker.ld" -o "bin/kernel.elf" -nostdlib "obj/boot.o" "obj/main.o" "obj/memory.o" "obj/fonts.o" "obj/text.o" "obj/input.o" "obj/idt.o" "obj/io.o" "obj/gdt.o" "obj/math.o" "obj/disk.o" "obj/iso9660.o"
if grub-file --is-x86-multiboot bin/kernel.elf; then 
        echo multiboot confirmed 
        mkdir -p isodir/boot/grub 
        cp bin/kernel.elf isodir/boot/kernel.bin 
        cp grub.cfg isodir/boot/grub/grub.cfg 
        grub-mkrescue -o dekos.iso isodir 
        qemu-system-i386 -cdrom dekos.iso -m 128 -monitor stdio -boot d 
else 
        echo the file is not multiboot 
fi 
 