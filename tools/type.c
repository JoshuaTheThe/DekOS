#ifdef __WIN32
#include "../dekoslibc/stdio.h"
#include "../dekoslibc/string.h"
#include "../dekoslibc/ini.h"
#else
#include <stdio.h>
#include <string.h>
#include <ini.h>
#endif

int main(USERID UserID, PID ParentProc, size_t argc, char **argv)
{
        char Message_1[] = " [ERROR] Incorrect usage, please provide path\n";
        char Message_2[] = " [ERROR] File does not exist or could not read from ";
        if (argc < 2)
        {
                write(Message_1, sizeof(Message_1));
                return 69;
        }

        char *FileData = ReadFile(argv[1]);
        
        if (!FileData)
        {
                write(Message_2, sizeof(Message_2));
                write(argv[1], strnlen(argv[1], 256));
                putc('\n');
                return 69;
        }

        write(FileData, 65536);
        free(FileData);
        return 0;
}
