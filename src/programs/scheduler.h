#ifndef SCHED_H
#define SCHED_H

#include <stddef.h>
#include <heap/alloc.h>
#include <tty/output/output.h>
#include <io.h>
#include <math.h>

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
} pid_t;

typedef struct
{
        uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip;
        uint32_t flags;
        uint32_t cs, ss, ds, es, fs, gs;
} regs_t;

typedef struct /* process */
{
        regs_t regs;
        uint32_t *stack, stack_size;
        uint8_t *program;
        bool active;
        bool valid;
        bool debugger_is_present;
        uint8_t name[PROC_NAME_LEN];
        uint32_t output[TTY_H][TTY_W];
} proc_t;

extern void Scheduler(void);
extern pid_t StartProcess(const char *Name, char **Args, int Argc, uint8_t *Program, uint32_t EntryPOffset, uint8_t *Stack, uint32_t StackLength);
extern bool KillProcess(pid_t Pid); /* Return True on fail */
extern void SchedInit(void);
extern bool SuspendProcess(pid_t pid);
extern void ListProcesses(void);
extern void transfer(void);
extern void SaveContext(void);
extern uint32_t CloneProcess(pid_t pid);
extern bool ProcessIsRunning(pid_t pid);
extern proc_t processes[MAX_PROCS];
extern pid_t current_pid;
extern uint32_t tick_counter;

#endif
