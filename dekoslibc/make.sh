clang -m32 -march=i386 -I. -c -ffreestanding -msoft-float -fno-builtin "ini.c" -o "ini.o" -Wall -Wextra
clang -m32 -march=i386 -I. -c -ffreestanding -msoft-float -fno-builtin "stdio.c" -o "stdio.o" -Wall -Wextra
clang -m32 -march=i386 -I. -c -ffreestanding -msoft-float -fno-builtin "string.c" -o "string.o" -Wall -Wextra
clang -m32 -march=i386 -I. -c "start.s" -o "start.o" -Wall -Wextra
