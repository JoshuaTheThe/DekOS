#ifndef SYS_H
#define SYS_H

#include <stdint.h>
#include <programs/scheduler.h>
#include <utils.h>

typedef enum InterruptFunction
{
        INT80_EXIT,
        INT80_WRITE,
        INT80_PUTCHAR,
} InterruptFunction_t;

void sysHang(void);
uint32_t sysReply(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3);

#endif
