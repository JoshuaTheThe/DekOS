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
        int handle = rreq(0); /* ResourceID of the frame buffer */
        if (handle == -1)
        {
                print("could not grab frame buffer\n");
                return -1;
        }

        char buf[1024*4],msg[128],r; /* in future, we will make a rinfo syscall, we know the width */
        for (size_t i = 0; i < 1024*4; ++i)
                buf[i] = 0xFF;
        for (size_t y = 0; y < 768; ++y)
        {
                /**
                 * int blit(int Handle,
        	 * char *buf,
        	 * size_t szbuf,
        	 * size_t bytes_to_copy,
        	 * size_t off)
                 */
                r=blit(handle, buf, 1024*4, 1024*4, 1024*4*y);
                snprintf(msg,128," [INFO] result=%d\n", r);
                print(msg);
        }

        gets(buf, 16);
        rgive(handle, 0); /* kernel pid == 0 always */
        return 0;
}
