        .global idtTimer
        .extern schedTimerHandler
idtTimer:
        cli
        call schedTimerHandler
        iret