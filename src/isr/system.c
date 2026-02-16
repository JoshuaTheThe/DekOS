#include <isr/system.h>
#include <memory/string.h>
#include <drivers/math.h>
#include <drivers/fs/file.h>
#include <drivers/dev/ps2/ps2.h>
#include <resource/main.h>

bool tty_needs_flushing = false;
extern schedProcess_t processes[];
extern int tick_counter;

extern void jumpToProc();
extern volatile DWORD mx, my, pmx, pmy, mbuttons;

extern KRNLRES grResources;

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

void sysBreakpoint(void)
{
        schedPid_t pid = schedGetCurrentPid();
        schedProcess_t *proc = schedGetProcess();

        printf("-- Breakpoint in %d was reached in %x:%x --\n", pid.num, proc->regs.cs, proc->regs.eip);
        printf("  EAX   %x\n", proc->regs.eax);
        printf("  EBX   %x\n", proc->regs.ebx);
        printf("  ECX   %x\n", proc->regs.ecx);
        printf("  EDX   %x\n", proc->regs.edx);
        printf("  ESP   %x\n", proc->regs.esp);
        printf("  EBP   %x\n", proc->regs.ebp);
        printf("  ESI   %x\n", proc->regs.esi);
        printf("  EDI   %x\n", proc->regs.edi);
        printf("-- Stack Trace --\n");
        for (size_t i = 0; i < 32; ++i)
        {
                printf(" %x\n", ((uint32_t *)proc->regs.esp)[-i * 4]);
        }
        proc->active = false;
}

/*
 * TODO - Factor into functions
 * */

uint32_t sysReply(void)
{
        schedPid_t pid = schedGetCurrentPid();
        schedProcess_t *proc = schedGetProcess();

        uint32_t syscall_num = *((uint32_t *)0x9000);
        uint32_t arg1 = *((uint32_t *)0x9004);
        uint32_t arg2 = *((uint32_t *)0x9008);
        uint32_t arg3 = *((uint32_t *)0x900C);
        uint32_t arg4 = *((uint32_t *)0x9018);
        uint32_t arg5 = *((uint32_t *)0x901C);

        // printf("Syscall: num=%d, arg1=%d, arg2=%d, arg3=%d\n", syscall_num, arg1, arg2, arg3);

        switch (syscall_num)
        {
                /* PROC MANAGEMENT */

                /* exit program */
        case EXIT:
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
        case YIELD:
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
        case GET_MY_PID:
                return pid.num;
                break;
        case GET_PARENTS_PID:
                return proc->parent.num;
                break;
        case CHECK_PROCESS:
        {
                if (arg1 >= MAX_PROCS)
                {
                        return 0;
                }

                return processes[arg1].valid;
        }

        case KILL_PROCESS:
        {
                if (arg1 < MAX_PROCS)
                {
                        schedPid_t p = {1, 1};
                        p.num = arg1;
                        schedKillProcess(p);
                }
                return 0;
        }

        case CREATE_PROCESS:
        {
                /* TODO - Allow drive switching by proc */
                DRIVE *Drive = SMGetDrive();
                const size_t sz = Drive->FileSize(Drive, (char *)arg1);
                bool iself = false;
                void *data = Drive->ReadFile(Drive, (char *)arg1);
                schedPid_t pid = elfLoadProgram(data, sz, &iself, proc->enactor, arg2, (char **)arg3);
                if (!iself)
                        return -1;
                return pid.num;
        }

        case RESUME_PROCESS:
        {
                if (arg1 < MAX_PROCS)
                {
                        schedPid_t p = {1, 1};
                        p.num = arg1;
                        schedResumeProcess(p);
                }
                return 0;
        }

        case SUSPEND_PROCESS:
        {
                if (arg1 < MAX_PROCS)
                {
                        schedPid_t p = {1, 1};
                        p.num = arg1;
                        schedSuspendProcess(p);
                }
                return 0;
        }

        /**
         * Life without UB and with safety is boring
         */
        case OPEN:
                return (uintptr_t)FOpen((const char *const)arg1, strnlen((const char *)arg1, MAX_PATH), arg2);
        case CLOSE:
                FClose((SYSFILE *)arg1);
                return 0;

                /* write a buffer to the process's tty buffer */
        case WRITE:
                FWrite((char *)arg1, arg2, arg3, (SYSFILE *)arg4);
                break;
        case READ:
                FRead((char *)arg1, arg2, arg3, (SYSFILE *)arg4);
                break;
        case GETCH:
                return ps2_keyboard_fetch(NULL);
        case PUTCH:
                putchar(arg1);
                return 0;

                /* put a singular character on the process's tty buffer */
                /* IPC */
                // void sendmsg(pid, msg, sizeof(msg)), max size of 1024
        case SEND:
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

                // int recvmsg(buffer, sizeof(buffer)), read the process's message buffer (recieve message), return PID
        case FETCH:
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

                return msg.sender_pid;
        }

                // bool msgrecv(pid), whether it has been read yet, -1 for any
        case UNREAD:
        {
                uint32_t sender_pid = arg1;
                IPCInbox_t *inbox = &processes[pid.num].inbox;

                for (int i = 0; i < inbox->count; i++)
                {
                        int index = (inbox->tail + i) % MAX_PENDING_MESSAGES;
                        if ((inbox->messages[index].sender_pid == sender_pid || sender_pid == (size_t)-1) &&
                            !inbox->messages[index].read)
                        {
                                return 1; // Has unread message
                        }
                }

                return 0; // No unread messages from this sender
        }

        case MOUSEX:
                return mx;
        case MOUSEY:
                return my;
        case MOUSEBTN:
                return mbuttons;

        case MALLOC:
                if (arg1 > 0 && arg1 < 1024 * 1024)
                {
                        return (uint32_t)malloc(arg1);
                }
                return 0;
        case FREE:
                free((void *)arg1);
                return 0;
        case USERNAME:
        {
                char *buf = (char *)arg1;
                size_t bufl = arg2;

                UserName(buf, bufl, proc->enactor);
                return (uintptr_t)buf;
        }
        break;
        case TICKS:
                return tick_counter;

        case RREQ:
        {
                KRNLRES *Res = ResourceGetFromRID(arg1, TRUE);
                if (Res)
                {
                        /**
                         * ResourceHandoverK only works from owner's end.
                         * */
                        Res->Owner = pid;
                        return Res->rid;
                }

                return -1;
        }
                return -1;
        case BLIT:
                /* blit(rid,buf,bufsz,off,cpysz) */
                {
                        KRNLRES *Res = ResourceGetFromRID(arg1, FALSE);
                        return ResourceBlitK(Res,
                                             (RAWPTR)arg2,
                                             (SIZE)arg3,
                                             (SIZE)arg4,
                                             (SIZE)arg5);
                }
                return -1;
        case RBLIT:
                /* rblit(rid,buf,bufsz,off,cpysz) */
                {
                        KRNLRES *Res = ResourceGetFromRID(arg1, FALSE);
                        return ResourceRBlitK(Res,
                                             (RAWPTR)arg2,
                                             (SIZE)arg3,
                                             (SIZE)arg4,
                                             (SIZE)arg5);
                }
                return -1;
        case RNEW:
                return -1;
        case RDEL:
                return -1;
        case RGIVE:
        {
                KRNLRES *Res = ResourceGetFromRID(arg1, FALSE);
                if (!Res)
                        return -1;
                PROCID Id = {.valid = 1, .num = arg2};
                Id.valid = schedValidatePid(Id);
                ResourceHandoverK(Res, Id);
                return 0;
        }
        case RSIZE:
        {
                KRNLRES *Res = ResourceGetFromRID(arg1, FALSE);
                if (!Res)
                        return -1;
                return Res->Region.size;
        }
        case RONHEAP:
        {
                KRNLRES *Res = ResourceGetFromRID(arg1, FALSE);
                if (!Res)
                        return -1;
                return Res->OnHeap;
        }
        case RTYPE:
        {
                KRNLRES *Res = ResourceGetFromRID(arg1, FALSE);
                if (!Res)
                        return -1;
                return Res->Type;
        }
        case ROWNER:
        {
                KRNLRES *Res = ResourceGetFromRID(arg1, FALSE);
                if (!Res)
                        return -1;
                return Res->Owner.num;
        }

        default:
                return -1;
                break;
        }
        return 0;
}

uint32_t syscall(uint32_t num, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
        uint32_t result;
        asm volatile(
            "int $0x80"
            : "=a"(result)
            : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4), "D"(arg5)
            : "memory");
        return result;
}
