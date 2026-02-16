#ifndef DEK_H
#define DEK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MAX_FILES (8192)
#define FILE_PRESENT (0x01)
#define FILE_READABLE (0x02)
#define FILE_WRITABLE (0x04)
#define FILE_EXECUTABLE (0x08)

#define DIR_WRITE (0x00)
#define DIR_READ (0x01)

#define MAX_PATH 256

typedef uint32_t USERID;
typedef uint32_t PID;

typedef enum InterruptFunction
{
        EXIT,
        GET_MY_PID,
        GET_PARENTS_PID,
        YIELD,
        CREATE_PROCESS,
        KILL_PROCESS,
        CHECK_PROCESS,
        RESUME_PROCESS,
        SUSPEND_PROCESS,
        USERNAME,
        TICKS,

        /** .. */

        OPEN = 0x10,
        CLOSE,
        READ,
        WRITE,
        GETCH,
        PUTCH,

        /** .. */

        SEND = 0x20,
        FETCH,
        UNREAD,
        MOUSEX,
	MOUSEY,
	MOUSEBTN,

        /** .. */

        MALLOC = 0x30,
        FREE,

	/** .. */

	RREQ = 0x40,
	BLIT,
        RBLIT,
	RNEW,
	RDEL,
	RGIVE,
        RSIZE,
        RONHEAP,
        RTYPE,
        ROWNER,
        RPROT,
} InterruptFunction_t;

/**
 * WARNING -- UNSAFE
 * THIS DATA IS SHARED, AND IS STORED **IN KERNEL STACK** (oh no)
*/
typedef struct _iobuf
{
	char *ptr;
	size_t remaining;
	size_t size;
	char *base;
	int flags;
        char *path;
} FILE;

FILE *open(char *path, int flags);
void close(FILE *fil);

void read(char *buf, size_t n, size_t size, FILE *fil);
void write(char *buf, size_t n, size_t size, FILE *fil);

void exit(int);
PID getmypid(void);
PID getparentspid(void);
void yield(PID);
PID createproc(char *,int,char **);
void killproc(PID);
int checkproc(PID);
void resumeproc(PID);
void suspendproc(PID);
void username(char *,int);
int ticks(void);
void send(PID,void *,int);
void fetch(PID,void *,int);
int unread(PID);

int mousex(void);
int mousey(void);
int mousebtn(void);

char getch(void);
void putch(char);

void *malloc(int);
void free(void *);

int rreq(int);
int blit(int,char*,size_t,size_t,size_t,bool);
int rnew(int);
int rdel(int);
int rgive(int,PID);
int rprot(int Handle, bool Protected);
void print(const char *s);

static inline uint32_t syscall(uint32_t num, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
        uint32_t result;
        __asm("cli;");
        asm volatile(
            "int $0x80"
            : "=a"(result)
            : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4), "D"(arg5)
            : "memory");
        __asm("sti;");
        return result;
}

static inline int rsize(int rID)
{
        return syscall(RSIZE, rID, 0, 0, 0, 0);
}

static inline int ronheap(int rID)
{
        return syscall(RONHEAP, rID, 0, 0, 0, 0);
}

static inline int rtype(int rID)
{
        return syscall(RTYPE, rID, 0, 0, 0, 0);
}

static inline PID rowner(int rID)
{
        return syscall(ROWNER, rID, 0, 0, 0, 0);
}

#endif
