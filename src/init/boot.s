        .section .multiboot
        .equ MULTIBOOT_HEADER_MAGIC          , 0x1BADB002
        .equ MULTIBOOT_HEADER_FLAGS          , 0x00000007   # VIDEO_MODE (4) + MEMORY_INFO (2) + PAGE_ALIGN (1)
        .equ MULTIBOOT_HEADER_CHECKSUM       , -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS) & 0xFFFFFFFF
        .align 16, 0
multiboot_header:
        .long   MULTIBOOT_HEADER_MAGIC
        .long   MULTIBOOT_HEADER_FLAGS
        .long   MULTIBOOT_HEADER_CHECKSUM
        .long   0, 0, 0, 0, 0
        .long   0
        .long   640
        .long   480
        .long   32
        .section .text
        .global _start
        .type _start, @function
_start:
	mov $stack_top, %esp
        cli
        push %ebx
        push %eax
	call main
	cli
1:	hlt
	jmp 1b
        .size _start, . - _start
        .section .bss
        .align 16
stack_bottom:
        .skip 65536*10
stack_top:
