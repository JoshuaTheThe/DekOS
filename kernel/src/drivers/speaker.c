/* PC Speaker */
#include <drivers/speaker.h>

void speakerPlay(uint32_t nFrequence)
{
        uint32_t d;
        uint8_t tmp;

        d = 1193180 / nFrequence;
        outb(0x43, 0xb6);
        outb(0x42, (uint8_t)(d));
        outb(0x42, (uint8_t)(d >> 8));
        tmp = inb(0x61);
        if (tmp != (tmp | 3))
        {
                outb(0x61, tmp | 3);
        }
}

void speakerStop(void)
{
        uint8_t tmp = inb(0x61) & 0xFC;
        outb(0x61, tmp);
}
