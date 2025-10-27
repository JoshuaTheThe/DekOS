#ifndef SHELL_H
#define SHELL_H

#include <heap/alloc.h>
#include <tty/input/input.h>
#include <tty/output/output.h>
#include <drivers/iso9660.h>
#include <programs/ex.h>

#define SHELL_KBD_BUFF_SIZE 64
#define SHELL_MAX_ARGS 8
#define __VER__ "1.0.0"

void shell(void);

#endif
