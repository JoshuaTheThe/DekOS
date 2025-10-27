#include <isr/system.h>

void hang(void)
{
        cli();
        while (1)
        {
                hlt();
        }
}

uint32_t sysreply(InterruptFunction_t function, uint32_t ebx, uint32_t ecx, uint32_t edx, uint32_t esi, uint32_t edi)
{
        switch (function)
        {
                case INT80_EXIT:
                {
                        KillProcess(current_pid);
                        sti();
                        while(1);
                        break;
                }
                case INT80_WRITE:
                        break;
        }
        return 0;
}
