#ifdef __WIN32
#include "./API/stdio.h"
#include "./API/string.h"
#include "./API/ini.h"
#else
#include <stdio.h>
#include <string.h>
#include <ini.h>
#endif

void print(const char *s)
{
        for (unsigned int i = 0; s[i]; ++i)
                putch(s[i]);
}

int main(USERID UserID, PID ParentProc, size_t argc, char **argv)
{
        for (size_t i = 0; i < 10; ++i)
        {
                print("Hello, World!\n");
        }
        return 69;
}
