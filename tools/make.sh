clang -m32 -march=i386 -I. -I../dekoslibc -c -ffreestanding -msoft-float -fno-builtin "type.c" -o "type.o" -Wall -Wextra

ld -m elf_i386 -o "type.elf" -nostdlib ../dekoslibc/start.o ../dekoslibc/stdio.o type.o ../dekoslibc/ini.o ../dekoslibc/string.o

cp *.elf ../fat32dir -rf