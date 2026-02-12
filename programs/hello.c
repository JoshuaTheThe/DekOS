#include <stdio.h>
#include <string.h>
#include <ini.h>

void print(const char *s)
{
        for (unsigned int i = 0; s[i]; ++i)
                putch(s[i]);
}

int main(uint32_t argc, char **argv, USERID UserID, PID ParentProc)
{
        for (size_t i = 0; i < 10; ++i)
        {
                print("Hello, World!\n");
        }
        return 69;
}
