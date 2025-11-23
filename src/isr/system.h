#ifndef SYS_H
#define SYS_H

#include <stdint.h>
#include <programs/scheduler.h>
#include <utils.h>

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
} InterruptFunction_t;

/* System Call Function Prototypes */

/**
 * syscall - Make a system call with up to 3 arguments
 * @num: System call number
 * @arg1: First argument
 * @arg2: Second argument
 * @arg3: Third argument
 * Returns: System call return value
 */
uint32_t syscall(uint32_t num, uint32_t arg1, uint32_t arg2, uint32_t arg3);

/**
 * sysReply - Main system call handler (called from assembly)
 * This is the C handler for int 0x80 interrupts
 */
uint32_t sysReply(void);

/**
 * sysHang - Halt the system indefinitely
 */
void sysHang(void);

/* Process Management Prototypes */

/**
 * exit - Terminate the current process
 * @status: Exit status code
 */
void exit(int status);

/**
 * yield - Yield execution to next available process
 * Returns: 0 on success, -1 on error
 */
int yield(void);

/**
 * yield_to - Yield execution to specific process
 * @pid: Target process ID
 * Returns: 0 on success, -1 if PID is invalid
 */
int yield_to(int pid);

/**
 * getpid - Get current process ID
 * Returns: Current process ID
 */
int getpid(void);

/**
 * getppid - Get parent process ID
 * Returns: Parent process ID
 */
int getppid(void);

/**
 * progexists - wait's for the given program to exit
 * Returns: whether the program exists, (true of false).
 */
int progexists(int pid);

/**
 * fork - Create a new process (clone current)
 * Returns: New PID in child, 0 in parent, -1 on error
 */
int fork(void);

/**
 * sleep - Sleep for specified number of timer ticks
 * @ticks: Number of timer ticks to sleep
 */
void sleep(uint32_t ticks);

/* I/O Operations Prototypes */

/**
 * write - Write buffer to process TTY
 * @str: String buffer to write
 * @len: Number of bytes to write
 * Returns: Number of bytes written, -1 on error
 */
int write(const char *str, uint32_t len);

/**
 * putch - Write single character to process TTY
 * @c: Character to write
 * Returns: 0 on success, -1 on error
 */
int putc(char c);

/**
 * kbhit - Check if keyboard key is available
 * Returns: 1 if key available, 0 otherwise
 */
int kbhit(void);

/**
 * getch - Get keyboard character (non-blocking)
 * Returns: Character code, 0 if no key available
 */
char getc(void);

/* Inter-Process Communication Prototypes */

/**
 * sendmsg - Send message to another process
 * @pid: Target process ID
 * @msg: Message buffer
 * @size: Message size in bytes
 * Returns: 0 on success, negative error code on failure
 */
int sendmsg(int pid, const char *msg, uint32_t size);

/**
 * recvmsg - Receive message from process inbox
 * @buffer: Buffer to store received message
 * @size: Size of buffer in bytes
 * Returns: Number of bytes received, -1 if no messages
 */
int recvmsg(char *buffer, uint32_t size);

/**
 * msgrecv - Check for unread messages from specific sender
 * @sender_pid: Sender process ID to check
 * Returns: true if unread messages exist, false otherwise
 */
bool msgrecv(int sender_pid);

#endif
