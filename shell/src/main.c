#ifdef __WIN32
#include "stdio.h"
#else
#include <stdio.h>
#endif

int strlen(const char *Str)
{
        size_t i;
        for (i = 0; Str[i]; ++i)
                ;
        return i;
}

void *ReadFile(const char *FilePath)
{
        Response resp;
        resp.Code = RESPONSE_READ_FILE;
        memcpy(resp.as.bytes, FilePath, strlen(FilePath));
        sendmsg(0, &resp, sizeof(Response));
        int pidn;
        while (1)
        {
                if (!msgrecv(-1))
                        continue;
                pidn = recvmsg(&resp, sizeof(resp));
                if (pidn != 0)
                        continue;
                break;
        }

        return resp.as.P;
}

void print(const char *s)
{
        for (unsigned int i = 0; s[i]; ++i)
                putc(s[i]);
}

int main(void)
{
        char *ConfigFile = ReadFile("system/system.ini");
        print(ConfigFile);
        free(ConfigFile);
        return 69;
}
