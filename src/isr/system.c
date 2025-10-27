#include <isr/system.h>

void sysHang(void)
{
        cli();
        while (1)
        {
                hlt();
        }
}

uint32_t sysReply(InterruptFunction_t function, uint32_t ebx, uint32_t ecx, uint32_t edx, uint32_t esi, uint32_t edi)
{
        schedPid_t pid = schedGetCurrentPid();
        (void)ebx;
        (void)ecx;
        (void)edx;
        (void)esi;
        (void)edi;

        switch (function)
        {
                case INT80_EXIT:
                {
                        schedKillProcess(pid);
                        sti();
                        while(1);
                        break;
                }
                case INT80_WRITE:
                        break;
        }
        return 0;
}
