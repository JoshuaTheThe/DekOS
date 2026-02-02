#include <programs/scheduler.h>

#include <memory/alloc.h>
#include <memory/string.h>
#include <tty/output/output.h>
#include <drivers/math.h>

schedProcess_t processes[MAX_PROCS];
schedProcess_t *current_process;
// gdtTssEntry_t *current_tss;
schedPid_t current_pid;
uint32_t tick_counter;
static uint32_t kStack[4096];

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

void schedInit(void)
{
	current_pid.num = 0;
	current_pid.valid = 1;
	memset(processes, 0, sizeof(processes));

	memcpy(processes[0].name, "Adam", 5);
	processes[0].valid = true;
	processes[0].active = true;
	processes[0].delete = false;
	processes[0].program = (uint8_t *)NULL;
	processes[0].stack = (uint8_t *)kStack;
	processes[0].stack_size = 4096;

	processes[0].regs.esp = (uint32_t)kStack + sizeof(kStack);
	processes[0].regs.ebp = processes[0].regs.esp;

	processes[0].regs.eax = 0;
	processes[0].regs.ebx = 0;
	processes[0].regs.ecx = 0;
	processes[0].regs.edx = 0;
	processes[0].regs.esi = 0;
	processes[0].regs.edi = 0;

	processes[0].regs.eip = (uint32_t)NULL;
	processes[0].regs.flags = 0x202; // Interrupts enabled
	processes[0].regs.cs = 0x08;
	processes[0].regs.ss = 0x10;
	processes[0].regs.ds = 0x10;
	processes[0].regs.es = 0x10;
	processes[0].regs.fs = 0x10;
	processes[0].regs.gs = 0x10;

	current_process = &processes[0];

	schedLoadContext();
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
			      uint8_t *Stack, uint32_t StackLength, schedPid_t parent)
{
	(void)Args;
	(void)Argc;
	pushf();
	cli();
	uint32_t flags = 0;
	int id = schedFindInvalidProcess();
	schedPid_t pid;

	// gdtTssEntry_t *tss = gdtGetTssEntries();

	if (Name && Program && id > 0)
	{
		pid.num = id;
		pid.valid = true;

		memset(&processes[id], 0, sizeof(schedProcess_t));
		memcpy(processes[id].name, Name, minu(sizeof(processes[id].name), strnlen(Name, 32)));
		processes[id].program = Program;
		processes[id].stack = Stack;
		processes[id].stack_size = StackLength;
		processes[id].regs.cs = 0x8;
		processes[id].regs.ds = 0x10;
		processes[id].regs.ss = 0x10;
		processes[id].regs.es = 0x10;
		processes[id].regs.fs = 0x10;
		processes[id].regs.gs = 0x10;
		processes[id].parent = parent;

		processes[id].regs.eip = (uint32_t)Program + EntryPOffset;
		processes[id].regs.esp = (uint32_t)(Stack + StackLength);

		processes[id].regs.eax = 0;
		processes[id].regs.ebx = 0;
		processes[id].regs.ecx = 0;
		processes[id].regs.edx = 0;
		processes[id].regs.esi = 0;
		processes[id].regs.edi = 0;
		processes[id].regs.ebp = processes[id].regs.esp; // Match ESP
		processes[id].regs.flags = 0x202;		 // Interrupts enabled + reserved bit
		processes[id].valid = true;
		processes[id].active = true;
		popf();
		return pid;
	}

	popf();
	pid.num = 0;
	pid.valid = false;
	return pid;
}

bool schedKillProcess(schedPid_t Pid)
{
	if (Pid.num == 0 || Pid.num > MAX_PROCS)
		return true;

	processes[Pid.num].active = false;
	processes[Pid.num].valid = false;
	processes[Pid.num].delete = true;

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

bool schedSwitch(uint32_t npid)
{
	if (npid > MAX_PROCS || !processes[npid].valid || !processes[npid].active || processes[npid].delete)
	{
		return true;
	}
	current_pid.num = npid;
	current_pid.valid = true;
	return false;
}

schedProcess_t *schedGetProcess(void)
{
	return &processes[current_pid.num];
}

int32_t schedFindNextTask(void)
{
	uint32_t start = current_pid.num;
	uint32_t next = start;

	do
	{
		next = (next + 1) % MAX_PROCS;

		if (processes[next].valid &&
		    processes[next].active &&
		    !processes[next].delete)
		{
			return (int32_t)next;
		}
	} while (next != start);

	return -1;
}

// uint32_t schedNextTSS(void)
// {
//         int32_t next_pid = schedFindNextTask();
//
//         if (next_pid != -1 && next_pid != current_pid.num)
//         {
//                 current_pid.num = next_pid;
//                 uint32_t tss_selector = (3 + next_pid) * 8;
//                 current_process = &processes[next_pid];
//                 current_tss = &gdtGetTssEntries()[next_pid];
//                 return tss_selector;
//         }
//
//         return (uint32_t)-1;
// }

void schedNextContext(void)
{
	/* find the next process, to load into the temp buffer */
	if (!current_pid.valid)
		current_pid.num = 0;
	uint32_t start = current_pid.num;
	uint32_t next = start;

	while (1)
	{
		next = (next + 1) % MAX_PROCS;
		if (processes[next].valid && processes[next].active)
		{
			current_pid.num = next;
			current_pid.valid = true;
			break;
		}
	}
}

void schedSaveContext(void)
{
	cli();
	/* save into current program */
	if (!current_pid.valid)
		while (1)
			;
	processes[current_pid.num].regs.eax = SCHED_EAX;
	processes[current_pid.num].regs.ebx = SCHED_EBX;
	processes[current_pid.num].regs.ecx = SCHED_ECX;
	processes[current_pid.num].regs.edx = SCHED_EDX;
	processes[current_pid.num].regs.esi = SCHED_ESI;
	processes[current_pid.num].regs.edi = SCHED_EDI;
	processes[current_pid.num].regs.esp = SCHED_ESP;
	processes[current_pid.num].regs.ebp = SCHED_EBP;

	processes[current_pid.num].regs.eip = SCHED_EIP;
	processes[current_pid.num].regs.flags = SCHED_FLAGS | 0x200;

	processes[current_pid.num].regs.cs = SCHED_CS;
	processes[current_pid.num].regs.ds = SCHED_DS;
	processes[current_pid.num].regs.ss = SCHED_SS;
	processes[current_pid.num].regs.es = SCHED_ES;
	processes[current_pid.num].regs.fs = SCHED_FS;
	processes[current_pid.num].regs.gs = SCHED_GS;
}

void schedLoadContext(void)
{
	cli();
	/* load current program into the temporary buffer */
	if (!current_pid.valid)
		while (1)
			;
	SCHED_EAX = processes[current_pid.num].regs.eax;
	SCHED_EBX = processes[current_pid.num].regs.ebx;
	SCHED_ECX = processes[current_pid.num].regs.ecx;
	SCHED_EDX = processes[current_pid.num].regs.edx;
	SCHED_ESI = processes[current_pid.num].regs.esi;
	SCHED_EDI = processes[current_pid.num].regs.edi;
	SCHED_ESP = processes[current_pid.num].regs.esp;
	SCHED_EBP = processes[current_pid.num].regs.ebp;

	SCHED_EIP = processes[current_pid.num].regs.eip;
	SCHED_FLAGS = processes[current_pid.num].regs.flags;

	SCHED_CS = processes[current_pid.num].regs.cs;
	SCHED_DS = processes[current_pid.num].regs.ds;
	SCHED_SS = processes[current_pid.num].regs.ss;
	SCHED_ES = processes[current_pid.num].regs.es;
	SCHED_FS = processes[current_pid.num].regs.fs;
	SCHED_GS = processes[current_pid.num].regs.gs;
}

void schedTimerHandler(void)
{
	++tick_counter;
	outb(0x20, 0x20);
}

schedPid_t schedGetKernelPid(void)
{
	return (schedPid_t){.num = 0, .valid = 1};
}

bool schedValidatePid(schedPid_t prId)
{
	if ((!prId.valid) || (prId.num < 0 || prId.num >= MAX_PROCS))
		return false;
	if (processes[prId.num].valid && processes[prId.num].active)
		return true;
	return false;
}
