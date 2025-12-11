#include <drivers/serial.h>

void SerialInit(void)
{
        outb(0x3F8 + 1, 0x00);
        outb(0x3F8 + 3, 0x80);
        outb(0x3F8 + 0, 0x03);
        outb(0x3F8 + 1, 0x00);
        outb(0x3F8 + 3, 0x03);
        outb(0x3F8 + 2, 0xC7);
        outb(0x3F8 + 4, 0x0B);
}

void SerialPut(char c)
{
        while ((inb(0x3F8 + 5) & 0x20) == 0)
                ;
        outb(0x3F8, c);
}

void SerialPrint(const char *str)
{
        while (*str)
                SerialPut(*str++);
}
