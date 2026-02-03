clang -m32 -march=i386 -I./src -I../dekoslibc -c -ffreestanding -msoft-float -fno-builtin "src/main.c" -o "obj/main.o" -Wall -Wextra

ld -m elf_i386 -o "bin/shell.elf" -nostdlib ../dekoslibc/start.o ../dekoslibc/stdio.o obj/main.o ../dekoslibc/ini.o ../dekoslibc/string.o
cp bin/*.elf ../fat32dir/system -rf
