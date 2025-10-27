        .global idtSysCall
        .extern sysReply
idtSysCall:
        cli
        pushl %edi
        pushl %esi
        pushl %edx
        pushl %ecx
        pushl %ebx
        pushl %eax
        call sysReply
        addl 4*6, %esp
        sti
        iret

        .global idtTimer
        .extern schedTick

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
        
idtTimer:
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
        jmp schedTick

        .global idtPageFault
idtPageFault:
        cli
        hlt
        jmp idtPageFault
        iret

        .global idtInvalidOpcodeHandler
        .global idtDivideByZeroHandler
        .global idtGeneralProtectionFaultHandler
        .global idtDefaultHandler
        .extern idtDefault
idtInvalidOpcodeHandler:
idtDivideByZeroHandler:
idtGeneralProtectionFaultHandler:
idtDefaultHandler:
        cli
        pusha
        call idtDefault
        popa
        sti
        iret