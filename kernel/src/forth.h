#ifndef FORTH_H

#include <utils.h>
#include <drivers/serial.h>

#define FORTH_STACK_SIZE 256

void push(int32_t val);
int32_t pop(void);
void forth(char *);

#endif
