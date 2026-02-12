#include <stdio.h>
#include <string.h>
#include <ini.h>

void puts(char *str, size_t len)
{
        for (size_t i = 0; i < len && str[i]; ++i)
        {
                putch(str[i]);
        }
}

int main(uint32_t argc, char **argv, USERID UserID, PID ParentProc)
{
        char Message_1[] = " [ERROR] Incorrect usage, please provide path\n";
        char Message_2[] = " [ERROR] File does not exist or could not read from ";
        if (argc < 2)
        {
                puts(Message_1, sizeof(Message_1));
                return 69;
        }

        FILE *File = open(argv[1], FILE_PRESENT | FILE_READABLE);
        char *FileData = File->base;
        
        if (!FileData)
        {
                puts(Message_2, sizeof(Message_2));
                puts(argv[1], strnlen(argv[1], 256));
                putch('\n');
                return 69;
        }

        puts(FileData, 65536);
        close(File);
        return 0;
}
