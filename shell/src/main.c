#include <stdio.h>

int main(void)
{
        write("Hello, World!\n\0", 16);
        sendmsg(0, "BALLS", 5);
        return 69;
}
