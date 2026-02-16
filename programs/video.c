#include <stdio.h>
#include <string.h>
#include <ini.h>

int main(uint32_t argc, char **argv, USERID UserID, PID ParentProc)
{
        (void)UserID;
        (void)ParentProc;
        (void)argc;
        (void)argv;
        int rid = rreq(0); /* ResourceID of the frame buffer */
        if (rid == -1)
        {
                print("could not grab frame buffer\n");
                return -1;
        }

        rprot(rid, true);
        char *buf = malloc(1024 * 768 * 4), msg[128], r;
        if (!buf)
        {
                rgive(rid, 0); /* kernel pid == 0 always */
                print("could not allocate buffer\n");
                return -1;
        }

        for (size_t y = 0; y < 768; ++y)
        {
                for (size_t x = 0; x < 1024; ++x)
                {
                        int pixel = y * 1024 + x;
                        uint32_t color = (x ^ y) * 0x01010101;
                        ((uint32_t *)buf)[pixel] = color;
                }
        }
        /**
         * int blit(int rid,
         * char *buf,
         * size_t szbuf,
         * size_t bytes_to_copy,
         * size_t off)
         * bool Dir
         */
        r = blit(rid, buf, 1024 * 768 * 4, 1024 * 768 * 4, 0, DIR_WRITE);
        snprintf(msg, 128, " [INFO] result=%d\n", r);
        print(msg);
        while (getch() != '\n');
        free(buf);
        rprot(rid, false);
        rgive(rid, 0); /* kernel pid == 0 always */
        return 0;
}
