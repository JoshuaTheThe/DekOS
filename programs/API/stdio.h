#ifndef STDIO_H
#define STDIO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>

#include "dek.h"

#define __int3() __asm__(".byte 0xCC\n");

int vsnprintf(char *str, size_t size, const char *fmt, va_list args);
int snprintf(char *str, size_t size, const char *fmt, ...);
int gets(char *b, int max);

#endif
