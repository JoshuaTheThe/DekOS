#include <programs/scheduler.h>

schedProcess_t processes[MAX_PROCS];
schedPid_t current_pid;
static uint32_t tick_counter;
static uint32_t kStack[4096];
void kernelTask(void);

int schedFindInvalidProcess(void)
{
        for (int i = 0; i < MAX_PROCS; ++i)
        {
                if (processes[i].valid == false)
                {
                        return i;
                }
        }

        return -1;
}

void schedTick(void)
{
        tick_counter++;
}

void schedInit(void)
{
        gdtTssEntry_t *tss = gdtGetTssEntries();
        current_pid.num = 0;
        current_pid.valid = 1;
        memset(processes, 0, MAX_PROCS * sizeof(schedProcess_t));

        memcpy(processes[0].name, "Adam", 5);
        processes[0].valid = true;
        processes[0].active = true;
        processes[0].program = (uint8_t *)kernelTask;
        processes[0].stack = (uint8_t *)kStack;
        processes[0].stack_size = 4096;
        processes[0].debugger_is_present = false;
        processes[0].tss = &tss[0];

        processes[0].tss->eip = (uint32_t)kernelTask;
        processes[0].tss->esp = (uint32_t)kStack + 4096 - 16;
        processes[0].kstack = malloc(4096);
        processes[0].tss->esp0 = processes[0].kstack + 4096 - 16;
        processes[0].tss->eflags = 0x200;
        processes[0].tss->cr3 = 0x00;
        processes[0].tss->cs = 0x08;
        processes[0].tss->ss = 0x10;
        processes[0].tss->ds = 0x10;
        processes[0].tss->es = 0x10;
        processes[0].tss->fs = 0x10;
        processes[0].tss->gs = 0x10;
        processes[0].tss->iomap_base = 0xFFFF;
}

bool schedSuspendProcess(schedPid_t pid)
{
        if (pid.num == 0 || !pid.valid || !processes[pid.num].active || !processes[pid.num].valid)
                return true;
        processes[pid.num].active = false;
        return false;
}

bool schedResumeProcess(schedPid_t pid)
{
        if (pid.num == 0 || !pid.valid || processes[pid.num].active || !processes[pid.num].valid)
                return true;
        processes[pid.num].active = true;
        return false;
}

void schedListProcesses(void)
{
        for (int i = 0; i < MAX_PROCS; ++i)
        {
                if (processes[i].valid)
                {
                        printf("%s : %s\n", processes[i].name, (processes[i].active) ? "running" : "suspended");
                }
        }
}

schedPid_t schedCreateProcess(const char *Name, char **Args, size_t Argc,
                              uint8_t *Program, uint32_t EntryPOffset,
                              uint8_t *Stack, uint32_t StackLength, bool ring0)
{
        int id = schedFindInvalidProcess();
        schedPid_t pid;

        gdtTssEntry_t *tss = gdtGetTssEntries();

        if (Name && Program && id > 0)
        {
                pid.num = id;
                pid.valid = true;

                memset(&processes[id], 0, sizeof(schedProcess_t));
                memcpy(processes[id].name, Name, minu(sizeof(processes[id].name), strlen(Name)));
                processes[id].program = Program;
                processes[id].stack = Stack;
                processes[id].stack_size = StackLength;
                processes[id].valid = true;
                processes[id].active = true;

                // Allocate kernel stack for this task (4KB)
                uint32_t *kernel_stack = malloc(4096);
                if (!kernel_stack)
                {
                        processes[id].valid = false;
                        pid.valid = false;
                        return pid;
                }

                uint32_t kernel_esp = (uint32_t)kernel_stack + 4096 - 16;
                uint32_t user_eip = (uint32_t)Program + EntryPOffset;
                uint32_t user_esp = (uint32_t)(Stack + StackLength - 16);

                // Initialize TSS for this task
                gdtInitTssForTask(id, kernel_esp, user_eip, user_esp, 0);

                // Store kernel stack pointer in the TSS pointer field for cleanup
                processes[id].tss = &tss[id];

                return pid;
        }

        pid.num = 0;
        pid.valid = false;
        return pid;
}

bool schedKillProcess(schedPid_t Pid)
{
        if (Pid.num == 0 || !Pid.valid || !processes[Pid.num].valid)
                return true;

        if (processes[Pid.num].kstack)
        {
                free((void *)(processes[Pid.num].kstack));
        }

        if (processes[Pid.num].stack)
        {
                free(processes[Pid.num].stack);
        }

        processes[Pid.num].active = false;
        processes[Pid.num].valid = false;

        if (processes[Pid.num].tss)
        {
                memset(processes[Pid.num].tss, 0, sizeof(gdtTssEntry_t));
        }

        processes[Pid.num].tss = NULL;
        memset(&processes[Pid.num], 0, sizeof(schedProcess_t));

        return false;
}

bool schedIsRunning(schedPid_t Pid)
{
        if (Pid.valid && Pid.num < MAX_PROCS)
        {
                return processes[Pid.num].active && processes[Pid.num].valid;
        }
        return false;
}

schedPid_t schedGetCurrentPid(void)
{
        return current_pid;
}

schedProcess_t schedGetProcess(void)
{
        return processes[current_pid.num];
}

uint32_t schedFindNextTask(void)
{
        uint32_t start = current_pid.num;
        uint32_t next = start;

        do
        {
                next = (next + 1) % MAX_PROCS;
                if (processes[next].valid && processes[next].active)
                {
                        return next;
                }
        } while (next != start);

        return -1;
}

uint32_t schedNextTSS(void)
{
        uint32_t next_pid = schedFindNextTask();

        if (next_pid != (uint32_t)-1 && next_pid != current_pid.num)
        {
                current_pid.num = next_pid;
                uint32_t tss_selector = (3 + next_pid) * 8;
                return tss_selector;
        }

        return (uint32_t)-1;
}

void schedTimerHandler(void)
{
        uint32_t next_tss = schedNextTSS();
        outb(0x20, 0x20);

        if (next_tss != (uint32_t)-1)
        {
                // Create a far jump descriptor on stack
                struct far_jump
                {
                        uint32_t eip;
                        uint16_t cs;
                        uint16_t unused;
                } __attribute__((packed)) jmp_desc = {0, (uint16_t)next_tss, 0};

                asm volatile(
                    "ljmp *%0"
                    :
                    : "m"(jmp_desc)
                    : "memory");
        }
}