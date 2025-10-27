#ifndef SYS_H
#define SYS_H

#include <stdint.h>
#include <programs/scheduler.h>
#include <utils.h>

typedef enum InterruptFunction
{
        INT80_EXIT,
        INT80_WRITE,
} InterruptFunction_t;

void sysHang(void);
uint32_t sysReply(InterruptFunction_t function, uint32_t ebx, uint32_t ecx, uint32_t edx, uint32_t esi, uint32_t edi);

#endif
