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

#endif
