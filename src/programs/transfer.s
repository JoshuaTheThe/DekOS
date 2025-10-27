        .global schedTransfer
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
schedTransfer:
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