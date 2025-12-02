#include <isr/system.h>
#include <programs/scheduler.h>
#include <tty/input/input.h>
#include <memory/alloc.h>
#include <memory/string.h>
#include <tty/output/output.h>
#include <tty/render/fonts.h>
#include <tty/render/render.h>
#include <drivers/math.h>

bool tty_needs_flushing=false;
extern schedProcess_t processes[];

extern void jumpToProc();

void sysHang(void)
{
        cli();
        while (1)
        {
                hlt();
        }
}

static bool ipcIsQueueFull(IPCInbox_t *inbox)
{
        return inbox->count >= MAX_PENDING_MESSAGES;
}

static bool ipcIsQueueEmpty(IPCInbox_t *inbox)
{
        return inbox->count == 0;
}

static void ipcEnqueueMessage(IPCInbox_t *inbox, IPCMessage_t *msg)
{
        if (ipcIsQueueFull(inbox))
                return;

        inbox->messages[inbox->head] = *msg;
        inbox->head = (inbox->head + 1) % MAX_PENDING_MESSAGES;
        inbox->count++;
}

static bool ipcDequeueMessage(IPCInbox_t *inbox, IPCMessage_t *msg)
{
        if (ipcIsQueueEmpty(inbox))
                return false;

        *msg = inbox->messages[inbox->tail];
        inbox->tail = (inbox->tail + 1) % MAX_PENDING_MESSAGES;
        inbox->count--;
        return true;
}

uint32_t sysReply(void)
{
        schedPid_t pid = schedGetCurrentPid();
        schedProcess_t *proc = schedGetProcess();

        uint32_t syscall_num = *((uint32_t *)0x1000);
        uint32_t arg1 = *((uint32_t *)0x1004);
        uint32_t arg2 = *((uint32_t *)0x1008);
        uint32_t arg3 = *((uint32_t *)0x100C);

        //printf("Syscall: num=%d, arg1=%d, arg2=%d, arg3=%d\n", syscall_num, arg1, arg2, arg3);

        switch (syscall_num)
        {
                /* PROC MANAGEMENT */

                /* exit program */
        case INT80_EXIT:
        {
                schedKillProcess(pid);
                sti();
                while (true)
                {
                        ;
                }
                break;
        }
                /* yield to the given pid */
        case INT80_YIELD:
        {
                schedSaveContext();
                if (!arg1)
                {
                        schedNextContext();
                        schedLoadContext();
                        __asm("jmp jumpToProc;");
                }
                else if (schedSwitch(arg1))
                {
                        schedLoadContext();
                        __asm("jmp jumpToProc;");
                }
                return -1;
                break;
        }
        case INT80_GET_PID:
                return pid.num;
                break;
        case INT80_GET_PARENT_PID:
                return proc->parent.num;
                break;
        case INT80_FORK:
                /* clone the process, and return here, but patch to other pid by setting EAX manually */
                return pid.num;
                break;

        case INT80_PID_EXISTS:
        {
                if (arg1 >= MAX_PROCS)
                {
                        return 0;
                }

                return processes[arg1].valid;
        }

        case INT80_RESERVED_1:
                break;

                /* sleep for a given amount of ticks - not implemented yet */
        case INT80_SLEEP:
                break;

                /* I/O */

                /* write a buffer to the process's tty buffer */
        case INT80_WRITE:
                if (arg1 != 0)
                {
                        char *_buf = (char *)arg1;
                        for (uint32_t i = 0; i < arg2; ++i)
                        {
                                putchar(_buf[i] /*, proc->tty, &proc->x, &proc->y*/);
                        }
                }
                break;

                /* put a singular character on the process's tty buffer */
        case INT80_PUTCH:
                putchar(arg1 /* , proc->tty, &proc->x, &proc->y */);
                break;

                /* test whether there is a key available */
        case INT80_KBHIT:
                return keyboardKeyPressed();

                /* get the ps/2 key pressed, need to use kbhit before to make sure that its actually gonna work */
        case INT80_GETCH:
                return keyboardFetch(NULL);
        case INT80_FLUSH:
                tty_needs_flushing=true;
                return 0;

                /* IPC */

                // void sendmsg(pid, msg, sizeof(msg)), max size of 1024
        case INT80_SENDMSG:
        {
                uint32_t target_pid = arg1;
                const char *message = (const char *)arg2;
                uint32_t msg_size = arg3;

                if (target_pid >= MAX_PROCS || !processes[target_pid].valid)
                {
                        return -1;
                }

                if (msg_size > MAX_MSG_SIZE)
                {
                        return -2;
                }

                IPCMessage_t ipc_msg;
                ipc_msg.sender_pid = pid.num;
                ipc_msg.receiver_pid = target_pid;
                ipc_msg.size = msg_size;
                ipc_msg.read = false;
                memcpy(ipc_msg.data, message, msg_size);

                IPCInbox_t *target_inbox = &processes[target_pid].inbox;

                if (ipcIsQueueFull(target_inbox))
                {
                        return -4;
                }

                ipcEnqueueMessage(target_inbox, &ipc_msg);
                return 0;
        }

                // void recvmsg(buffer, sizeof(buffer)), read the process's message buffer (recieve message)
        case INT80_RECVMSG:
        {
                char *buffer = (char *)arg1;
                uint32_t buffer_size = arg2;

                IPCInbox_t *inbox = &processes[pid.num].inbox;
                IPCMessage_t msg;

                if (!ipcDequeueMessage(inbox, &msg))
                {
                        return -1;
                }

                uint32_t copy_size = minu(msg.size, buffer_size);
                memcpy(buffer, msg.data, copy_size);

                return copy_size;
        }

                // bool msgrecv(pid), whether it has been read yet
        case INT80_MSGRECV:
        {
                uint32_t sender_pid = arg1;
                IPCInbox_t *inbox = &processes[pid.num].inbox;

                for (int i = 0; i < inbox->count; i++)
                {
                        int index = (inbox->tail + i) % MAX_PENDING_MESSAGES;
                        if (inbox->messages[index].sender_pid == sender_pid &&
                            !inbox->messages[index].read)
                        {
                                return 1; // Has unread message
                        }
                }

                return 0; // No unread messages from this sender
        }

        case INT80_ALLOC:
                if (arg1 > 0 && arg1 < 1024*1024)
                {
                        return (uint32_t)malloc(arg1);
                }
                return 0;
        case INT80_UNALLOC:
                free((void*)arg1);
                return 0;
        default:
                return -1;
                break;
        }
        return 0;
}

uint32_t syscall(uint32_t num, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
        uint32_t result;
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

void sleep(uint32_t ticks)
{
        syscall(INT80_SLEEP, ticks, 0, 0);
}

/* I/O Operations */
int write(const char *str, uint32_t len)
{
        return syscall(INT80_WRITE, (uint32_t)str, len, 0);
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
int sendmsg(int pid, const char *msg, uint32_t size)
{
        return syscall(INT80_SENDMSG, pid, (uint32_t)msg, size);
}

int recvmsg(char *buffer, uint32_t size)
{
        return syscall(INT80_RECVMSG, (uint32_t)buffer, size, 0);
}

bool msgrecv(int sender_pid)
{
        return syscall(INT80_MSGRECV, sender_pid, 0, 0) != 0;
}
