#include <stdint.h>
#include <stdbool.h>

uintptr_t __stack_chk_guard = 0;

static uint32_t simple_rand(void)
{
        static uint32_t seed = 0x12345678;
        seed = seed * 1103515245 + 12345;
        return seed;
}

void __stack_protector_init(void)
{
        if (__stack_chk_guard == 0)
        {
                uintptr_t esp;
                __asm__ volatile("mov %%esp, %0" : "=r"(esp));
                uint32_t time = 0;

                __stack_chk_guard = simple_rand();
                __stack_chk_guard ^= esp;
                __stack_chk_guard ^= time;

                if (__stack_chk_guard == 0 || __stack_chk_guard == 0xdeadbeef)
                {
                        __stack_chk_guard = 0x0ff1e3d4;
                }
        }
}

void __stack_chk_fail(void)
{
        const char *msg = "*** STACK SMASHING DETECTED ***\n";
        volatile uint16_t *vga = (uint16_t *)0xB8000;
        for (int i = 0; msg[i]; i++)
        {
                vga[i] = (0x04 << 8) | msg[i];
        }

        __asm__ volatile("cli");
        while (1)
        {
                __asm__ volatile("hlt");
        }
}

__attribute__((constructor)) static void __stack_chk_init_auto(void)
{
        __stack_protector_init();
}
