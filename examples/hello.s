        bits 32
        org 0x0000 ; we dont want the assembler to be dumb
section .text
program:
        ; custom relocatable format
.header:
        ; sign, NULL, major ver, minor ver, patch
        db "EXENNEA", 0, 1, 1, 0
        ; relocation count, relocations offset
        dd 1, .relocations
        ; .text origin, .text size
        dd .text, .relocations-.text
        ; .funtable origin, function count
        dd .funtable, 1
        ; stack size
        dd 1024
.text:
.main:
        mov eax, 8
        db 0xBB
.rel0:
        dd .hello
        mov ecx, 15
        int 0x80
        mov eax, 0
        mov ebx, 0
        int 0x80
        jmp $
.relocations:
        dd .rel0
.funtable:
        db "main", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        dd .main
.hello:
        db "Hello, World!", 10, 0