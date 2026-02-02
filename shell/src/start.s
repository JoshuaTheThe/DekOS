        .extern main
        .extern exit
        .global _start
_start:
        mov $stack_top, %esp
        and $-16, %esp        # 16-byte alignment
        call main
        push %eax
        call exit
        .section .bss
        .align 16
stack:
        .skip 16384
stack_top: