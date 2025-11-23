        .global idtKeyboardHandler
        .global idtCascadeHandler
        .global idtCOM2Handler
        .global idtCOM1Handler
        .global idtLPT2Handler
        .global idtFloppyHandler
        .global idtLPT1Handler
        .global idtRTCHandler
        .global idtMouseHandler
        .global idtSB16Handler
        # stub
idtKeyboardHandler:
idtCascadeHandler:
idtCOM2Handler:
idtCOM1Handler:
idtLPT2Handler:
idtFloppyHandler:
idtLPT1Handler:
idtRTCHandler:
idtMouseHandler:
idtSB16Handler:
        cli
        push $0xFE
        call idtDefault
        add $4, %esp
        sti
        sti
        iret
