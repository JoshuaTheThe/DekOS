#ifndef STDIO_H
#define STDIO_H

#include <stdint.h>
#include <stdbool.h>

bool msgrecv(int sender_pid);
int recvmsg(char *buffer, uint32_t size);
int sendmsg(int pid, const char *msg, uint32_t size);
char getc(void);
int kbhit(void);
int putc(char c);
int write(const char *str, uint32_t len);
void sleep(uint32_t ticks);
int progexists(int pid);
int fork(void);
int getppid(void);
int getpid(void);
int yield_to(int pid);
int yield(void);
void exit(int status);

char getchar(void);

typedef enum InterruptFunction
{
        /* PROCESS MANAGEMENT */
        INT80_EXIT,
        INT80_YIELD,
        INT80_GET_PID,
        INT80_GET_PARENT_PID,
        INT80_FORK,
        INT80_PID_EXISTS,
        INT80_RESERVED_1,
        INT80_SLEEP,

        /* I/O */
        INT80_WRITE,
        INT80_PUTCH,
        INT80_KBHIT,
        INT80_GETCH,
        INT80_FLUSH,

        /* IPC */
        INT80_SENDMSG, /* void sendmsg(pid, msg, sizeof(msg)), max size of 1024 */
        INT80_RECVMSG, /* void recvmsg(buffer, sizeof(buffer)), read the process's message buffer (recieve message) */
        INT80_MSGRECV, /* bool msgrecv(pid), whether it has been read yet */

        /* MEMORY */
        INT80_ALLOC,   /* BASICALLY MALLOC */
        INT80_UNALLOC, /* BASICALLY FREE */

        /* RESOURCE */
        INT80_RELEASE_RESOURCE,
        /* WM */
        INT80_CREATE_WINDOW,
        INT80_CREATE_ELEMENT,
        INT80_ISFOCUSED,
} InterruptFunction_t;

#endif
