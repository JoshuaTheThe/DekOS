        bits 32
        section .text
        global _start
_start:
        mov edi, 10
.loop:
        mov eax, 8
        mov ebx, hello
        mov ecx, hello.end-hello
        xor edx, edx
        int 0x80

        dec edi
        jnz .loop

        xor eax, eax
        xor ebx, ebx
        int 0x80

        section .data
hello:
        db "Hello, World, from ELF!", 10
.end:

