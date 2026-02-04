#ifndef SCHED_H
#define SCHED_H

#include <utils.h>
#include <io.h>
#include <user/main.h>

#define PROC_NAME_LEN 32
#define MAX_PROCS 16

typedef size_t PID;

#define SCHED_EAX   *((uint32_t *)(0x9000 + (4 * 0)))
#define SCHED_EBX   *((uint32_t *)(0x9000 + (4 * 1)))
#define SCHED_ECX   *((uint32_t *)(0x9000 + (4 * 2)))
#define SCHED_EDX   *((uint32_t *)(0x9000 + (4 * 3)))
#define SCHED_ESP   *((uint32_t *)(0x9000 + (4 * 4)))
#define SCHED_EBP   *((uint32_t *)(0x9000 + (4 * 5)))
#define SCHED_ESI   *((uint32_t *)(0x9000 + (4 * 6)))
#define SCHED_EDI   *((uint32_t *)(0x9000 + (4 * 7)))
#define SCHED_EIP   *((uint32_t *)(0x9000 + (4 * 8)))
#define SCHED_CS    *((uint32_t *)(0x9000 + (4 * 9)))
#define SCHED_DS    *((uint32_t *)(0x9000 + (4 * 10)))
#define SCHED_ES    *((uint32_t *)(0x9000 + (4 * 11)))
#define SCHED_SS    *((uint32_t *)(0x9000 + (4 * 12)))
#define SCHED_FS    *((uint32_t *)(0x9000 + (4 * 13)))
#define SCHED_GS    *((uint32_t *)(0x9000 + (4 * 14)))
#define SCHED_FLAGS *((uint32_t *)(0x9000 + (4 * 15)))

#define MAX_MSG_SIZE 256
#define MAX_PENDING_MESSAGES 255

typedef struct
{
        uint32_t sender_pid;
        uint32_t receiver_pid;
        uint32_t size;
        uint8_t data[MAX_MSG_SIZE];
        bool read;
} IPCMessage_t;

typedef struct
{
        IPCMessage_t messages[MAX_PENDING_MESSAGES];
        uint8_t head;
        uint8_t tail;
        uint8_t count;
        uint8_t _padding;
} IPCInbox_t;

typedef struct
{
        bool valid : 1;
        uint32_t num : 31;
} schedPid_t;

typedef schedPid_t PROCID;

typedef struct
{
        uint32_t eax, ebx, ecx, edx, esp, ebp, esi, edi, eip, cs, ds, es, ss, fs, gs, flags;
} schedRegs_t;

typedef struct
{
        IPCInbox_t inbox;
        uint8_t name[PROC_NAME_LEN];
        uint32_t stack_size;
        uint8_t *program, *stack;
        schedRegs_t regs;
        schedPid_t parent;
        uint32_t x, y;
        bool valid;
        bool active;
        bool delete;
        bool debugger_is_present;

        char **argv;
        int argc;

        USERID enactor;
} schedProcess_t;

typedef schedProcess_t EINSTANCE;

void schedNext(void);
int schedFindInvalidProcess(void);
void schedTick(void);
void schedInit(void);
bool schedSuspendProcess(schedPid_t pid);
bool schedResumeProcess(schedPid_t pid);
void schedListProcesses(void);
schedPid_t schedCreateProcess(const char *Name, char **Args, size_t Argc, uint8_t *Program, uint32_t EntryPOffset, uint8_t *Stack, uint32_t StackLength, schedPid_t parent, USERID User);
bool schedKillProcess(schedPid_t Pid);
bool schedIsRunning(schedPid_t Pid);
schedPid_t schedGetCurrentPid(void);
schedProcess_t *schedGetProcess(void);
void schedLoadContext(void);
bool schedSwitch(uint32_t npid);
void schedSaveContext(void);
void schedNextContext(void);
schedPid_t schedGetKernelPid(void);
bool schedValidatePid(schedPid_t prId);
schedProcess_t *schedGetProcessN(schedPid_t Pid);

#endif
