#include <stdio.h>
#include <string.h>

extern int main(void);

char getchar(void)
{
        for (;;)
        {
                if (kbhit())
                        return getc();
        }
}

unsigned int syscall(unsigned int num, unsigned int arg1, unsigned int arg2, unsigned int arg3)
{
        unsigned int result;
        asm volatile(
            "int $0x80"
            : "=a"(result)
            : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3)
            : "memory");
        return result;
}

/* Process Management */
void exit(int status)
{
        syscall(INT80_EXIT, status, 0, 0);
        while (1)
                ;
}

int yield(void)
{
        return syscall(INT80_YIELD, 0, 0, 0);
}

int yield_to(int pid)
{
        return syscall(INT80_YIELD, pid, 0, 0);
}

int getpid(void)
{
        return syscall(INT80_GET_PID, 0, 0, 0);
}

int getppid(void)
{
        return syscall(INT80_GET_PARENT_PID, 0, 0, 0);
}

int fork(void)
{
        return syscall(INT80_FORK, 0, 0, 0);
}

int progexists(int pid)
{
        return syscall(INT80_PID_EXISTS, pid, 0, 0);
}

void sleep(unsigned int ticks)
{
        syscall(INT80_SLEEP, ticks, 0, 0);
}

/* I/O Operations */
int write(const char *str, unsigned int len)
{
        return syscall(INT80_WRITE, (unsigned int)str, len, 0);
}

int putc(char c)
{
        return syscall(INT80_PUTCH, c, 0, 0);
}

int kbhit(void)
{
        return syscall(INT80_KBHIT, 0, 0, 0);
}

char getc(void)
{
        return (char)syscall(INT80_GETCH, 0, 0, 0);
}

/* IPC */
int sendmsg(int pid, const char *msg, unsigned int size)
{
        return syscall(INT80_SENDMSG, pid, (unsigned int)msg, size);
}

int recvmsg(char *buffer, unsigned int size)
{
        return syscall(INT80_RECVMSG, (unsigned int)buffer, size, 0);
}

bool msgrecv(int sender_pid)
{
        return syscall(INT80_MSGRECV, sender_pid, 0, 0) != 0;
}

void *malloc(unsigned int size)
{
        return (void *)syscall(INT80_ALLOC, size, 0, 0);
}

void free(void *p)
{
        syscall(INT80_UNALLOC, (uint32_t)p, 0, 0);
}

void *ReadFile(const char *FilePath)
{
        Response resp;
        resp.Code = RESPONSE_READ_FILE;
        memcpy(resp.as.bytes, FilePath, strlen(FilePath));
        sendmsg(0, &resp, sizeof(Response));
        int pidn;
        while (1)
        {
                if (!msgrecv(-1))
                        continue;
                pidn = recvmsg(&resp, sizeof(resp));
                if (pidn != 0)
                        continue;
                break;
        }

        return resp.as.P;
}
