#ifndef SCHED_H
#define SCHED_H

#include <stddef.h>
#include <memory/alloc.h>
#include <memory/string.h>
#include <tty/output/output.h>
#include <io.h>
#include <drivers/math.h>

#define MAX_PROCS 4096
#define PROC_NAME_LEN 32

#define SCHED_EAX *((uint32_t *)(0x1000 + (4 * 0)))
#define SCHED_EBX *((uint32_t *)(0x1000 + (4 * 1)))
#define SCHED_ECX *((uint32_t *)(0x1000 + (4 * 2)))
#define SCHED_EDX *((uint32_t *)(0x1000 + (4 * 3)))
#define SCHED_ESP *((uint32_t *)(0x1000 + (4 * 4)))
#define SCHED_EBP *((uint32_t *)(0x1000 + (4 * 5)))
#define SCHED_ESI *((uint32_t *)(0x1000 + (4 * 6)))
#define SCHED_EDI *((uint32_t *)(0x1000 + (4 * 7)))
#define SCHED_EIP *((uint32_t *)(0x1000 + (4 * 8)))
#define SCHED_CS *((uint32_t *)(0x1000 + (4 * 9)))
#define SCHED_DS *((uint32_t *)(0x1000 + (4 * 10)))
#define SCHED_ES *((uint32_t *)(0x1000 + (4 * 11)))
#define SCHED_SS *((uint32_t *)(0x1000 + (4 * 12)))
#define SCHED_FS *((uint32_t *)(0x1000 + (4 * 13)))
#define SCHED_GS *((uint32_t *)(0x1000 + (4 * 14)))
#define SCHED_EFL *((uint32_t *)(0x1000 + (4 * 15)))

typedef struct
{
        bool valid : 1;
        uint32_t num : 31;
} schedPid_t;

typedef struct
{
        uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip;
        uint32_t flags;
        uint32_t cs, ss, ds, es, fs, gs;
} schedRegisters_t;

typedef struct /* process */
{
        schedRegisters_t regs;
        uint32_t stack_size;
        uint32_t output[TTY_H][TTY_W];
        uint8_t *program, *stack;
        uint8_t name[PROC_NAME_LEN];
        bool active;
        bool valid;
        bool debugger_is_present;
} schedProcess_t;

void schedNext(void);
int schedFindInvalidProcess(void);
int schedCloneProcess(schedPid_t pid);
void schedSaveContext(void);
void schedTick(void);
void schedInit(void);
bool schedSuspendProcess(schedPid_t pid);
bool schedResumeProcess(schedPid_t pid);
void schedListProcesses(void);
schedPid_t schedCreateProcess(const char *Name, char **Args, size_t Argc, uint8_t *Program, uint32_t EntryPOffset, uint8_t *Stack, uint32_t StackLength);
bool schedKillProcess(schedPid_t Pid);
bool schedIsRunning(schedPid_t Pid);
void schedTransfer(void);
schedPid_t schedGetCurrentPid(void);

#endif
