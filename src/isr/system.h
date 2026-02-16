#ifndef SYS_H_
#define SYS_H_

#include <utils.h>
#include <programs/scheduler.h>
#include <programs/elf/elf.h>

typedef enum InterruptFunction
{
        EXIT,
        GET_MY_PID,
        GET_PARENTS_PID,
        YIELD,
        CREATE_PROCESS,
        KILL_PROCESS,
        CHECK_PROCESS,
        RESUME_PROCESS,
        SUSPEND_PROCESS,
        USERNAME,
        TICKS,

        /** .. */

        OPEN = 0x10,
        CLOSE,
        READ,
        WRITE,
        GETCH,
        PUTCH,

        /** .. */

        SEND = 0x20,
        FETCH,
        UNREAD,
        MOUSEX,
        MOUSEY,
        MOUSEBTN,

        /** .. */

        MALLOC = 0x30,
        FREE,

        /** .. */

        RREQ = 0x40,
        BLIT,
        RNEW,
        RDEL,
        RGIVE,
        RSIZE,
        RONHEAP,
        RTYPE,
        ROWNER,
} InterruptFunction_t;

/* System Call Function Prototypes */

/**
 * syscall - Make a system call with up to 3 arguments
 * @num: System call number
 * @arg1: First argument
 * @arg2: Second argument
 * @arg3: Third argument
 * Returns: System call return value
 */
uint32_t syscall(uint32_t num, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5);

#endif
