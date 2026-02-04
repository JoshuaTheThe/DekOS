        bits 32
        org 0x0000 ; we dont want the assembler to be dumb
section .text
program:
        ; custom relocatable format
.header:
        ; sign, NULL, major ver, minor ver, patch
        db "EXENNEA", 0, 1, 1, 0
        ; relocation count, relocations offset
        dd 0, .relocations
        ; .text origin, .text size
        dd .text, .relocations-.text
        ; .funtable origin, function count
        dd .funtable, 1
        ; stack size
        dd 64
.text:
.main:
        mov eax, 9
        mov ebx, 66
        int 0x80
        mov eax, 0
        mov ebx, 0
        int 0x80
        jmp $
.relocations:
.funtable:
        db "main", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        dd .main
        