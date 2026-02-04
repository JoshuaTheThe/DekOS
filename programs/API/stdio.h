#ifndef STDIO_H
#define STDIO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>

#define __int3() __asm__ (".byte 0xCC\n");

typedef enum
{
        RESPONSE_WTF = 0,
        RESPONSE_HANDOVER_RESOURCE,
        RESPONSE_READ_FILE,
        RESPONSE_CREATE_PROC,
        RESPONSE_OK = 200,
} ResponseCode;

typedef struct __attribute__((__packed__))
{
        unsigned int Code;

        union
        {
                void *P;
                char bytes[200];
        } as;
} Response;

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

        INT80_GET_USER_NAME,
} InterruptFunction_t;

typedef size_t USERID;
typedef size_t PID;

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
void *malloc(unsigned int size);
void free(void *p);
void *ReadFile(const char *FilePath);
PID CreateProcess(char **argv, int argc);
int gets(char *b, int max);
int vsnprintf(char *str, size_t size, const char *fmt, va_list args);
int snprintf(char *str, size_t size, const char *fmt, ...);
void getusername(char *buf, size_t bufl);

#endif
