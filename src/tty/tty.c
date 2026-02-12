#include <tty/tty.h>
#include <drivers/math.h>

FILE _iob[STDFILECNT];

static char TTYBuff[TTY_SIZE];
static size_t tail = 0, head = 0;

size_t ttyIndex(size_t i) { return (i + 1) % TTY_SIZE; }

void *ttyRead(DRIVE *tty)
{
        if (head == tail)
                return 0;

        size_t count;
        if (head > tail)
        {
                count = head - tail;
                memcpy(tty->BufferA, &TTYBuff[tail], count);
        }
        else
        {
                size_t first = TTY_SIZE - tail;
                memcpy(tty->BufferA, &TTYBuff[tail], first);
                memcpy(tty->BufferA + first, &TTYBuff[0], head);
                count = first + head;
        }

        tail = head;
        tty->SizeOfOp = count;
        return tty->BufferA;
}

void *ttyWrite(DRIVE *tty)
{
        for (size_t i = 0; i < tty->SizeOfOp; i++)
                putchar(tty->BufferA[i]);
        return tty->BufferA;
}

int ttyMain(size_t argc, char **argv, USERID UserID, PID ParentProc)
{
        printf("Hello, World!\n");
        while (true)
        {
                // char c = ps2_getchar();
                // size_t next = (head + 1) % TTY_SIZE;
                // if (next != tail)
                // {
                //         TTYBuff[head] = c;
                //         head = next;
                // }
                // yield(0);
        }
}

void *read(char *buf, size_t len)
{
        void *BufferA = SMReadFrom(0, _iob[STDIN_FILENO].Drive);
        memcpy(buf, BufferA, minu(_iob[STDIN_FILENO].Drive->SizeOfOp, minu(len, 4096)));
        return buf;
}

int gets(char *buf, size_t len)
{
        if (!buf || len == 0)
                return 0;

        size_t i = 0, ch;

        while (i < len - 1)
        {
                ch = (size_t)ps2_getchar();

                if ((char)ch == '\b' || (char)ch == 127)
                {
                        if (i > 0)
                        {
                                i--;
                                putchar('\b');
                                putchar(' ');
                                putchar('\b');
                                buf[i] = '\0';
                        }
                }
                else if (ch == '\n' || ch == '\r')
                {
                        putchar((uint8_t)ch);
                        break;
                }
                else if (ch >= 32 && ch <= 126)
                {
                        putchar((uint8_t)ch);
                        buf[i] = (char)ch;
                        i++;
                }
        }

        buf[i] = '\0';
        return i;
}
