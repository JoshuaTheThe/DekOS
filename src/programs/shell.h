#ifndef SHELL_H
#define SHELL_H

#include <memory/alloc.h>
#include <memory/string.h>
#include <tty/input/input.h>
#include <tty/output/output.h>
#include <drivers/iso9660.h>
#include <programs/ex.h>
#include <programs/delangue.h>

#define SHELL_KBD_BUFF_SIZE 64
#define SHELL_MAX_ARGS 8
#define __VER__ "1.1.0"

void shell(void);

#endif
