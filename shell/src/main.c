#include <stdio.h>

int main(void)
{
        write("Hello, World!\n\0", 16);
        sendmsg(0, "BALLS", 5);
        char buf[4096];
        int pidn;
        while (1)
        {
                if (!msgrecv(-1))
                        continue;
                pidn = recvmsg(buf, 4096);
                buf[4095] = 0;
                if (pidn != 0)
                        continue;
                write(" [INFO] Response from kernel: ", 31);
                write(buf, 4096);
                write("\n", 1);
        }
        return 69;
}
