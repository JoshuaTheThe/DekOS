#include <stdio.h>
#include <string.h>
#include <ini.h>

int main(uint32_t argc, char **argv, USERID UserID, PID ParentProc)
{
        (void)UserID;
        (void)ParentProc;
        char Message_1[] = " [ERROR] Incorrect usage, please provide path\n";
        char Message_2[] = " [ERROR] File does not exist or could not read from ";
        if (argc < 2)
        {
                print(Message_1);
                return 69;
        }

        FILE *File = open(argv[1], FILE_PRESENT | FILE_READABLE);
        if (!File)
        {
                print(Message_2);
                print(argv[1]);
                putch('\n');
                return 69;       
        }
        
        char *FileData = File->base;
        
        if (!FileData)
        {
                print(Message_2);
                print(argv[1]);
                putch('\n');
                return 69;
        }

        print(FileData);
        putch('\n');
        close(File);
        return 0;
}
