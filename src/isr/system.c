#include <isr/system.h>

void sysHang(void)
{
        cli();
        while (1)
        {
                hlt();
        }
}

uint32_t sysReply(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
        schedPid_t pid = schedGetCurrentPid();
        schedProcess_t proc = schedGetProcess();
        switch (syscall_num)
        {
        case INT80_EXIT:
        {
                printf("Process %d exiting\n", pid.num);
                schedKillProcess(pid);
                sti();
                while (1)
                        ;
                break;
        }
        case INT80_WRITE:
                if (arg1 != 0)
                {
                        printf("%s", (char *)arg1);
                }
                break;
        case INT80_PUTCHAR:
                putchar(arg1);
                break;
        default:
                printf("Unknown syscall: %d\n", syscall_num);
                break;
        }
        return 0;
}
