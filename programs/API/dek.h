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

#define MAX_PATH 128

typedef uint32_t USERID;
typedef int32_t PID;

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

        /** .. */
        DLOAD = 0x50,
        DFIND,
        DUNLOAD,
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

void exit(int);
void print(const char *s);

static inline uint32_t syscall(uint32_t num, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
        uint32_t result;
        __asm("cli;");
        __asm("pushl %ebx");
        __asm("pushl %ecx");
        __asm("pushl %edx");
        __asm("pushl %esi");
        __asm("pushl %edi");
        __asm("pushl %ebp");
        asm volatile(
            "int $0x80"
            : "=a"(result)
            : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4), "D"(arg5)
            : "memory");
        __asm("sti;");
        __asm("popl %ebp");
        __asm("popl %edi");
        __asm("popl %esi");
        __asm("popl %edx");
        __asm("popl %ecx");
        __asm("popl %ebx");
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

static inline PID getmypid(void)
{
        return syscall(GET_MY_PID, 0, 0, 0, 0, 0);
}

static inline PID getparentspid(void)
{
        return syscall(GET_PARENTS_PID, 0, 0, 0, 0, 0);
}

static inline void yield(PID pid)
{
        syscall(YIELD, pid, 0, 0, 0, 0);
}

static inline PID createproc(char *path, int arg_c, char **arg_v)
{
        return syscall(CREATE_PROCESS, (uintptr_t)path, arg_c, (uintptr_t)arg_v, 0, 0);
}

static inline void killproc(PID pid)
{
        syscall(KILL_PROCESS, pid, 0, 0, 0, 0);
}

static inline int checkproc(PID pid)
{
        return syscall(CHECK_PROCESS, pid, 0, 0, 0, 0);
}

static inline void resumeproc(PID pid)
{
        syscall(RESUME_PROCESS, pid, 0, 0, 0, 0);
}

static inline void suspendproc(PID pid)
{
        syscall(SUSPEND_PROCESS, pid, 0, 0, 0, 0);
}

static inline void username(char *buf, int max)
{
        syscall(USERNAME, (uintptr_t)buf, max, 0, 0, 0);
}

static inline int ticks(void)
{
        return syscall(TICKS, 0, 0, 0, 0, 0);
}

static inline void send(PID rec, void *buf, int siz)
{
        syscall(SEND, rec, (uintptr_t)buf, siz, 0, 0);
}

static inline void fetch(PID sen, void *buf, int siz)
{
        syscall(FETCH, sen, (uintptr_t)buf, siz, 0, 0);
}

static inline int unread(PID sen)
{
        return syscall(UNREAD, sen, 0, 0, 0, 0);
}

static inline int mousex(void)
{
        return syscall(MOUSEX, 0, 0, 0, 0, 0);
}

static inline int mousey(void)
{
        return syscall(MOUSEY, 0, 0, 0, 0, 0);
}

static inline int mousebtn(void)
{
        return syscall(MOUSEBTN, 0, 0, 0, 0, 0);
}

static inline void *malloc(int siz)
{
        return (void*)syscall(MALLOC, siz, 0, 0, 0, 0);
}

static inline void free(void *p)
{
        syscall(FREE, (uintptr_t)p, 0, 0, 0, 0);
}

static inline FILE *open(char *path, int flags)
{
        return (FILE *)syscall(OPEN, (uintptr_t)path, flags, 0, 0, 0);
}

static inline void close(FILE *fil)
{
        syscall(CLOSE, (uintptr_t)fil, 0, 0, 0, 0);
}

static inline void read(char *buf, size_t n, size_t size, FILE *fil)
{
        syscall(READ, (uintptr_t)buf, n, size, (uintptr_t)fil, 0);
}

static inline void write(char *buf, size_t n, size_t size, FILE *fil)
{
        syscall(WRITE, (uintptr_t)buf, n, size, (uintptr_t)fil, 0);
}

static inline char getch(void)
{
        return syscall(GETCH, 0, 0, 0, 0, 0);
}

static inline void putch(char chr)
{
        syscall(PUTCH, chr, 0, 0, 0, 0);
}

static inline int rreq(int Handle)
{

        return syscall(RREQ, Handle, 0, 0, 0, 0);
}

static inline int blit(int Handle,
	 char *buf,
	 size_t szbuf,
	 size_t bytes_to_copy,
	 size_t off, bool Direction)
{
	return syscall(BLIT + Direction,
		Handle, (size_t)buf, szbuf, bytes_to_copy, off);
}

static inline int rnew(int Type)
{
	return syscall(RNEW, Type, 0, 0, 0, 0);
}

static inline int rdel(int Handle)
{
	return syscall(RDEL, Handle, 0, 0, 0, 0);
}

static inline int rgive(int Handle, PID dest)
{
	return syscall(RGIVE, Handle, dest, 0, 0, 0);
}

static inline int rprot(int Handle, bool Protected)
{
	return syscall(RPROT, Handle, Protected, 0, 0, 0);
}

static inline int dlload(const char *path)
{
	return syscall(DLOAD, (uintptr_t)path, 0, 0, 0, 0);
}

static inline void *dlfind(int RID, const char *sym)
{
	return (void *)syscall(DFIND, RID, (uintptr_t)sym, 0, 0, 0);
}

static inline int dlunload(int RID)
{
	return syscall(DUNLOAD, RID, 0, 0, 0, 0);
}


#endif
