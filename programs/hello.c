#include <stdio.h>
#include <string.h>
#include <ini.h>

int main(uint32_t argc, char **argv, USERID UserID, PID ParentProc)
{
        (void)argc;
        (void)argv;
        (void)UserID;
        (void)ParentProc;
        for (size_t i = 0; i < 10; ++i)
        {
                print("Hello, World!\n");
        }
        return 69;
}
