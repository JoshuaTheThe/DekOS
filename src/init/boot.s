        .section .multiboot
        .align 16
multiboot_header:
        .long   0x1BADB002              /* Magic */
        .long   0x00000007              /* Flags: PAGE_ALIGN + MEMORY_INFO + VIDEO_MODE */
        .long   -(0x1BADB002 + 0x07)    /* Checksum */
        .long   0, 0, 0, 0, 0           /* Unused fields */
        .long   0                        /* Mode type: linear graphics */
        .long   1024                     /* Width */
        .long   768                      /* Height */
        .long   32                       /* Depth */

        .section .text
        .global _start
        .type _start, @function
_start:
        mov $stack_top, %esp
        xor %ebp, %ebp
        push %ebx
        push %eax
        call __stack_protector_init
        call kmain
        cli
        1:  hlt
        jmp 1b
        .size _start, . - _start
        .section .bss
        .align 16
stack_bottom:
        .skip 65536*4
stack_top: