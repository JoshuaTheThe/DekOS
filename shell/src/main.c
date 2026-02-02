#ifdef __WIN32
        #include "stdio.h"
        #include "string.h"
        #include "ini.h"
#else
        #include <stdio.h>
        #include <string.h>
        #include <ini.h>
#endif

void print(const char *s)
{
        for (unsigned int i = 0; s[i]; ++i)
                putc(s[i]);
}

int main(USERID UserID, PID ParentProc, int ecx, int edx)
{
        print("Hello, World!\n");
        return 69;
}
