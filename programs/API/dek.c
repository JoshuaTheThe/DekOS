#include <dek.h>
#include <string.h>

void exit(int code)
{
        syscall(EXIT, code, 0, 0, 0, 0);
}

PID getmypid(void)
{
        return syscall(GET_MY_PID, 0, 0, 0, 0, 0);
}

PID getparentspid(void)
{
        return syscall(GET_PARENTS_PID, 0, 0, 0, 0, 0);
}

void yield(PID pid)
{
        syscall(YIELD, pid, 0, 0, 0, 0);
}

PID createproc(char *path, int arg_c, char **arg_v)
{
        return syscall(CREATE_PROCESS, (uintptr_t)path, arg_c, (uintptr_t)arg_v, 0, 0);
}

void killproc(PID pid)
{
        syscall(KILL_PROCESS, pid, 0, 0, 0, 0);
}

int checkproc(PID pid)
{
        return syscall(CHECK_PROCESS, pid, 0, 0, 0, 0);
}

void resumeproc(PID pid)
{
        syscall(RESUME_PROCESS, pid, 0, 0, 0, 0);
}

void suspendproc(PID pid)
{
        syscall(SUSPEND_PROCESS, pid, 0, 0, 0, 0);
}

void username(char *buf, int max)
{
        syscall(USERNAME, (uintptr_t)buf, max, 0, 0, 0);
}

int ticks(void)
{
        return syscall(TICKS, 0, 0, 0, 0, 0);
}

void send(PID rec, void *buf, int siz)
{
        syscall(SEND, rec, (uintptr_t)buf, siz, 0, 0);
}

void fetch(PID sen, void *buf, int siz)
{
        syscall(FETCH, sen, (uintptr_t)buf, siz, 0, 0);
}

int unread(PID sen)
{
        return syscall(UNREAD, sen, 0, 0, 0, 0);
}

int mousex(void)
{
        return syscall(MOUSEX, 0, 0, 0, 0, 0);
}

int mousey(void)
{
        return syscall(MOUSEY, 0, 0, 0, 0, 0);
}

int mousebtn(void)
{
        return syscall(MOUSEBTN, 0, 0, 0, 0, 0);
}

void *malloc(int siz)
{
        return (void*)syscall(MALLOC, siz, 0, 0, 0, 0);
}

void free(void *p)
{
        syscall(FREE, (uintptr_t)p, 0, 0, 0, 0);
}

FILE *open(char *path, int flags)
{
        return (FILE *)syscall(OPEN, (uintptr_t)path, flags, 0, 0, 0);
}

void close(FILE *fil)
{
        syscall(CLOSE, (uintptr_t)fil, 0, 0, 0, 0);
}

void read(char *buf, size_t n, size_t size, FILE *fil)
{
        syscall(READ, (uintptr_t)buf, n, size, (uintptr_t)fil, 0);
}

void write(char *buf, size_t n, size_t size, FILE *fil)
{
        syscall(WRITE, (uintptr_t)buf, n, size, (uintptr_t)fil, 0);
}

char getch(void)
{
        return syscall(GETCH, 0, 0, 0, 0, 0);
}

void putch(char chr)
{
        syscall(PUTCH, chr, 0, 0, 0, 0);
}

int rreq(int Handle)
{

        return syscall(RREQ, Handle, 0, 0, 0, 0);
}

int blit(int Handle,
	 char *buf,
	 size_t szbuf,
	 size_t bytes_to_copy,
	 size_t off, bool Direction)
{
	return syscall(BLIT + Direction,
		Handle, (size_t)buf, szbuf, bytes_to_copy, off);
}

int rnew(int Type)
{
	return syscall(RNEW, Type, 0, 0, 0, 0);
}

int rdel(int Handle)
{
	return syscall(RDEL, Handle, 0, 0, 0, 0);
}

int rgive(int Handle, PID dest)
{
	return syscall(RGIVE, Handle, dest, 0, 0, 0);
}

int rprot(int Handle, bool Protected)
{
	return syscall(RPROT, Protected, 0, 0, 0, 0);
}

void print(const char *s)
{
        for (unsigned int i = 0; s[i]; ++i)
                putch(s[i]);
}

int dlload(const char *path)
{
	return syscall(DLOAD, (uintptr_t)path, 0, 0, 0, 0);
}

int dlfind(int RID, const char *sym)
{
	return syscall(DFIND, RID, (uintptr_t)sym, 0, 0, 0);
}

int dlunload(int RID)
{
	return syscall(DUNLOAD, RID, 0, 0, 0, 0);
}

