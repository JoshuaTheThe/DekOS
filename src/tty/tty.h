#ifndef _TTY_H
#define _TTY_H

#include <drivers/fs/storage.h>
#include <drivers/dev/ps2/ps2.h>
#include <memory/string.h>
#include <user/main.h>
#include <programs/scheduler.h>
#define TTY_SIZE 1024

#define STDIN_FILENO (0)
#define STDOUT_FILENO (1)
#define STDERR_FILENO (2)

#define STDFILECNT (3)

#define stdin	(&_iob[STDIN_FILENO])
#define stdout	(&_iob[STDOUT_FILENO])
#define stderr	(&_iob[STDERR_FILENO])

typedef struct
{
        DRIVE *Drive;
} FILE;

extern FILE _iob[STDFILECNT];

int ttyMain(USERID UserID, PID ParentProc, size_t argc, char **argv);
void *ttyWrite(DRIVE *tty);
void *ttyRead(DRIVE *tty);
void *read(char *buf, size_t len);
int gets(char *buf, size_t len);

#endif
