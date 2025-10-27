#include <programs/scheduler.h>

proc_t processes[MAX_PROCS];
pid_t current_pid;
uint32_t tick_counter;

static void ScheduleNext(void)
{
        current_pid.num = (current_pid.num + 1) % MAX_PROCS;
        while (!processes[current_pid.num].active || !processes[current_pid.num].valid)
        {
                current_pid.num = (current_pid.num + 1) % MAX_PROCS;
        }

        SCHED_EAX = processes[current_pid.num].regs.eax;
        SCHED_EBX = processes[current_pid.num].regs.ebx;
        SCHED_ECX = processes[current_pid.num].regs.ecx;
        SCHED_EDX = processes[current_pid.num].regs.edx;
        SCHED_ESI = processes[current_pid.num].regs.esi;
        SCHED_EDI = processes[current_pid.num].regs.edi;
        SCHED_ESP = processes[current_pid.num].regs.esp;
        SCHED_EBP = processes[current_pid.num].regs.ebp;
        SCHED_EIP = processes[current_pid.num].regs.eip;
        SCHED_EFL = processes[current_pid.num].regs.flags | 0x200;
        SCHED_CS = processes[current_pid.num].regs.cs;
        SCHED_SS = processes[current_pid.num].regs.ss;
        SCHED_ES = processes[current_pid.num].regs.es;
        SCHED_FS = processes[current_pid.num].regs.fs;
        SCHED_GS = processes[current_pid.num].regs.gs;
        SCHED_DS = processes[current_pid.num].regs.ds;
        transfer();
}

int FindFreeProcessSlot()
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

uint32_t CloneProcess(pid_t pid)
{
        if (!pid.valid || pid.num >= MAX_PROCS)
                return (uint32_t)-1;
        proc_t *orig = &processes[pid.num];
        proc_t proc;
        memcpy(&proc, orig, sizeof(proc_t));
        proc.stack = malloc(orig->stack_size);
        if (!proc.stack)
        {
                return (uint32_t)-1;
        }

        proc.stack_size = orig->stack_size;
        memcpy(proc.stack, orig->stack, proc.stack_size);
        uint32_t esp_offset = (uint32_t)(orig->regs.esp) - (uint32_t)(orig->stack);
        proc.regs.esp = (uint32_t)(proc.stack) + esp_offset;
        int new_pid_num = FindFreeProcessSlot();
        if (new_pid_num < 0)
        {
                free(proc.stack);
                return (uint32_t)-1;
        }
        processes[new_pid_num] = proc;
        processes[new_pid_num].regs.eax = 0;
        return (uint32_t)new_pid_num;
}

void SaveContext(void)
{
        processes[current_pid.num].regs.eax = SCHED_EAX;
        processes[current_pid.num].regs.ebx = SCHED_EBX;
        processes[current_pid.num].regs.ecx = SCHED_ECX;
        processes[current_pid.num].regs.edx = SCHED_EDX;
        processes[current_pid.num].regs.esi = SCHED_ESI;
        processes[current_pid.num].regs.edi = SCHED_EDI;
        processes[current_pid.num].regs.esp = SCHED_ESP;
        processes[current_pid.num].regs.ebp = SCHED_EBP;
        processes[current_pid.num].regs.eip = SCHED_EIP;
        processes[current_pid.num].regs.flags = SCHED_EFL | 0x200;
        processes[current_pid.num].regs.cs = 0x08;
        processes[current_pid.num].regs.ss = 0x10;
        processes[current_pid.num].regs.es = 0x10;
        processes[current_pid.num].regs.fs = 0x10;
        processes[current_pid.num].regs.gs = 0x10;
        processes[current_pid.num].regs.ds = 0x10;
}

void Scheduler(void)
{
        tick_counter++;
        if (current_pid.valid)
        {
                SaveContext();
                ScheduleNext();
        }
}

void SchedInit(void)
{
        current_pid.num = 0;
        current_pid.valid = 1;
        memset(processes, 0, MAX_PROCS * sizeof(proc_t));
        memcpy(processes[0].name, "Adam", 5);
        processes[0].valid = true;
        processes[0].active = true;
        processes[0].program = (uint8_t *)NULL;
        processes[0].stack = (uint32_t *)NULL;
        processes[0].stack_size = 0;
        processes[0].debugger_is_present = false;
}

bool SuspendProcess(pid_t pid)
{
        if (pid.num == 0 || !pid.valid || !processes[pid.num].active || !processes[pid.num].valid)
                return true;
        processes[pid.num].active = false;
        return false;
}

bool ResumeProcess(pid_t pid)
{
        if (pid.num == 0 || !pid.valid || processes[pid.num].active || !processes[pid.num].valid)
                return true;
        processes[pid.num].active = true;
        return false;
}

void ListProcesses(void)
{
        for (int i = 0; i < MAX_PROCS; ++i)
        {
                if (processes[i].valid)
                {
                        printf("%s : %s\n", processes[i].name, (processes[i].active) ? "running" : "suspended");
                }
        }
}

pid_t StartProcess(const char *Name, char **Args, int Argc, uint8_t *Program, uint32_t EntryPOffset, uint8_t *Stack, uint32_t StackLength)
{
        int id = FindFreeProcessSlot();
        pid_t pid;
        if (Name && Program)
        {
                pid.num = id;
                pid.valid = true;
                memset(&processes[id].regs, 0, sizeof(regs_t));
                memcpy(processes[id].name, Name, min(sizeof(processes[id].name), strlen(Name)));
                processes[id].program = Program;
                processes[id].valid = 1;
                processes[id].active = 1;
                processes[id].regs.eip = (uint32_t)Program + EntryPOffset;
                processes[id].regs.eax = (uint32_t)Args;
                processes[id].regs.ebx = Argc;
                processes[id].regs.edi = (uint32_t)Program;
                processes[id].regs.flags = 0x200;
                processes[id].regs.cs = 0x08;
                processes[id].regs.ss = 0x10;
                processes[id].regs.ds = 0x10;
                processes[id].regs.es = 0x10;
                processes[id].regs.fs = 0x10;
                processes[id].regs.gs = 0x10;
                processes[id].regs.esp = (uint32_t)Stack + (StackLength - 4);
                processes[id].regs.ebp = (uint32_t)Stack + (StackLength - 4);
                processes[id].stack = (uint32_t *)Stack;
                processes[id].stack_size = StackLength;
                id++;
                return pid;
        }

        pid.num = 0;
        pid.valid = 0;
        return pid;
}

bool KillProcess(pid_t Pid)
{
        if (Pid.num == 0 || !Pid.valid || !processes[Pid.num].active || !processes[Pid.num].valid)
                return true;
        processes[Pid.num].active = false;
        processes[Pid.num].valid = false;
        return 0;
}

bool ProcessIsRunning(pid_t Pid)
{
        if (Pid.valid && Pid.num < MAX_PROCS)
        {
                return processes[Pid.num].active && processes[Pid.num].valid;
        }
        return false;
}
