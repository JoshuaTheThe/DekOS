        .global systemcall
        .global invalid_opcode_handler
        .global timer_interrupt_handler
        .global divide_by_zero_handler
        .global general_protection_fault_handler
        .global page_fault_handler
        .global transfer
        .extern tick
        .extern scheduler
        .extern sysreply

        .equ TEMP_REG  , 0x1000
        .equ TEMP_EAX  , TEMP_REG+(0*4)
        .equ TEMP_EBX  , TEMP_REG+(1*4)
        .equ TEMP_ECX  , TEMP_REG+(2*4)
        .equ TEMP_EDX  , TEMP_REG+(3*4)
        .equ TEMP_ESP  , TEMP_REG+(4*4)
        .equ TEMP_EBP  , TEMP_REG+(5*4)
        .equ TEMP_ESI  , TEMP_REG+(6*4)
        .equ TEMP_EDI  , TEMP_REG+(7*4)
        .equ TEMP_EIP  , TEMP_REG+(8*4)
        .equ TEMP_CS   , TEMP_REG+(9*4)
        .equ TEMP_DS   , TEMP_REG+(10*4)
        .equ TEMP_ES   , TEMP_REG+(11*4)
        .equ TEMP_SS   , TEMP_REG+(12*4)
        .equ TEMP_FS   , TEMP_REG+(13*4)
        .equ TEMP_GS   , TEMP_REG+(14*4)
        .equ TEMP_FLGS , TEMP_REG+(15*4)
        .equ TEMP_STCK , 0x4000
systemcall:
        cli
        mov %esp, [TEMP_ESP]
        mov %esp, TEMP_STCK
        pushl %edi
        pushl %edx
        pushl %ebx
        pushl %eax
        call sysreply
        popl %ebx
        popl %ebx
        popl %edx
        popl %edi
        mov [TEMP_ESP], %esp
        sti
        iret
transfer:
        cli
        # Segments
        movw [TEMP_DS],%ax
        movw [TEMP_SS],%bx
        movw [TEMP_ES],%cx
        movw [TEMP_FS],%dx
        movw [TEMP_GS],%si
        movw %ax,%ds
        movw %bx,%ss
        movw %cx,%es
        movw %dx,%fs
        movw %si,%gs

        # Registers
        movl [TEMP_EAX],%eax
        movl [TEMP_EBX],%ebx
        movl [TEMP_ECX],%ecx
        movl [TEMP_EDX],%edx
        movl [TEMP_EBP],%ebp
        movl [TEMP_ESI],%esi
        movl [TEMP_EDI],%edi
        movl [TEMP_ESP],%esp

        # Interrupt Frame
        pushl [TEMP_FLGS]
        pushl [TEMP_CS]
        pushl [TEMP_EIP]
        iret
timer_interrupt_handler:
        cli
        # Registers & CS
        movl %eax,[TEMP_EAX]
        movl %ebx,[TEMP_EBX]
        movl %ecx,[TEMP_ECX]
        movl %edx,[TEMP_EDX]
        movl %ebp,[TEMP_EBP]
        movl %esi,[TEMP_ESI]
        movl %edi,[TEMP_EDI]
        popl [TEMP_EIP]
        popl [TEMP_CS]
        popl [TEMP_FLGS]
        movl %esp,[TEMP_ESP]
        # Segments
        xorl %eax,%eax
        xorl %ebx,%ebx
        xorl %ecx,%ecx
        xorl %edx,%edx
        xorl %esi,%esi
        movw %ds,%ax
        movw %es,%bx
        movw %ss,%cx
        movw %fs,%dx
        movw %gs,%si
        movw %ax,[TEMP_DS]
        movw %bx,[TEMP_ES]
        movw %cx,[TEMP_SS]
        movw %dx,[TEMP_FS]
        movw %si,[TEMP_GS]
        movw $0x10,%ax
        movw %ax,%ds
        movw %ax,%ss
        movw %ax,%es
        movw %ax,%fs
        movw %ax,%gs
        jmp scheduler
invalid_opcode_handler:
divide_by_zero_handler:
general_protection_fault_handler:
page_fault_handler:
        cli
        hlt
        jmp page_fault_handler
        iret