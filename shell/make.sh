clang -m32 -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin "src/main.c" -o "obj/main.o" -Wall -Wextra
clang -m32 -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin "src/stdio.c" -o "obj/stdio.o" -Wall -Wextra
clang -m32 -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin "src/start.s" -o "obj/start.o" -Wall -Wextra
clang -m32 -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin "src/ini.c" -o "obj/ini.o" -Wall -Wextra
clang -m32 -march=i386 -I./src -c -ffreestanding -msoft-float -fno-builtin "src/string.c" -o "obj/string.o" -Wall -Wextra

ld -m elf_i386 -o "bin/shell.elf" -nostdlib obj/start.o obj/stdio.o obj/main.o obj/ini.o obj/string.o
