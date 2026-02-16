#include <dek.h>
#include <string.h>

void exit(int code)
{
        syscall(EXIT, code, 0, 0, 0, 0);
}

void print(const char *s)
{
        for (unsigned int i = 0; s[i]; ++i)
                putch(s[i]);
}
