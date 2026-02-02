#include <forth.h>
#include <memory/string.h>

static int32_t stack[FORTH_STACK_SIZE];
static int32_t sp = 0;

void itos(int value, char *str, int base)
{
        char *ptr = str;
        char *start = str;
        int is_negative = 0;

        if (value < 0 && base == 10)
        {
                is_negative = 1;
                value = -value;
        }

        do
        {
                int digit = value % base;
                *ptr++ = (digit > 9) ? (digit - 10) + 'a' : digit + '0';
                value /= base;
        } while (value != 0);

        if (is_negative)
        {
                *ptr++ = '-';
        }

        *ptr = '\0';

        char *end = ptr - 1;
        while (start < end)
        {
                char temp = *start;
                *start++ = *end;
                *end-- = temp;
        }
}

bool isdigit(char chr)
{
        return chr >= '0' && chr <= '9';
}

int atoi(const char *const str, size_t *len)
{
        int k = 0, i = 0;
        while (str[i] >= '0' && str[i] <= '9')
        {
                k *= 10;
                k += str[i++] - '0';
        }

        if (len)
        {
                *len = i;
        }

        return k;
}

void push(int32_t val)
{
        if (sp < FORTH_STACK_SIZE)
                stack[sp++] = val;
}

int32_t pop(void)
{
        if (sp > 0)
                return stack[--sp];
#ifdef FORTH_DEBUG
        SerialPrint("WARNING: Stack underflow\r\n");
#endif
        return 0;
}

void executeToken(char **buf)
{
        if (!(*buf) || !(**buf))
                return;
        char *tok = *buf;
        if (isdigit(*tok))
        {
                size_t l;
                push(atoi(tok, &l));
                *buf += l - 1;
        }
        else if (*tok == 'l')
        {
#ifdef FORTH_DEBUG
                SerialPrint("INFO: Grabbing\r\n");
#endif
                uint32_t index = pop();
                push(stack[index % FORTH_STACK_SIZE]);
        }
        else if (*tok == 's')
        {
                int32_t index = pop();
                int32_t value = pop();
                stack[index % FORTH_STACK_SIZE] = value;
        }
        else if (*tok == '+')
        {
#ifdef FORTH_DEBUG
                SerialPrint("INFO: Adding\r\n");
#endif
                int32_t b = pop();
                int32_t a = pop();
                push(a + b);
        }
        else if (*tok == '@')
        {
#ifdef FORTH_DEBUG
                SerialPrint("INFO: Grabbing PC\r\n");
#endif
                push((int32_t)tok + 1);
        }
        else if (*tok == '!')
        {
#ifdef FORTH_DEBUG
                SerialPrint("INFO: Jumping\r\n");
#endif
                *buf = (char *)pop();
                return;
        }
        else if (*tok == '?')
        {
#ifdef FORTH_DEBUG
                SerialPrint("INFO: Jumping?\r\n");
#endif
                if ((int32_t)pop() > 0)
                        *buf = (char *)pop();
                return;
        }
        else if (*tok == '-')
        {
#ifdef FORTH_DEBUG
                SerialPrint("INFO: Subtracting\r\n");
#endif
                int32_t b = pop();
                int32_t a = pop();
                push(a - b);
        }
        else if (*tok == '*')
        {
#ifdef FORTH_DEBUG
                SerialPrint("INFO: Multiplying\r\n");
#endif
                int32_t b = pop();
                int32_t a = pop();
                push(a * b);
        }
        else if (*tok == '/')
        {
#ifdef FORTH_DEBUG
                SerialPrint("INFO: Dividing\r\n");
#endif
                int32_t b = pop();
                int32_t a = pop();
                push(a / b);
        }
        else if (*tok == '.')
        {
#ifdef FORTH_DEBUG
                SerialPrint("INFO: Printing\r\n");
#endif
                char str[16];
                itos(pop(), str, 10);
                SerialPrint(str);
                SerialPrint("\r\n");
        }
        else if (*tok == ',')
        {
#ifdef FORTH_DEBUG
                SerialPrint("INFO: Reading Char\r\n");
#endif
                push(SerialRead());
        }
        else if (*tok == '$')
        {
#ifdef FORTH_DEBUG
                SerialPrint("INFO: Printing Char\r\n");
#endif
                SerialPut(pop());
        }
        else if (*tok == 'd')
        {
#ifdef FORTH_DEBUG
                SerialPrint("INFO: Duplicating\r\n");
#endif
                int32_t val = pop();
                push(val);
                push(val);
        }
        else if (*tok == '^')
        {
#ifdef FORTH_DEBUG
                SerialPrint("INFO: Swapping\r\n");
#endif
                int32_t a = pop();
                int32_t b = pop();
                push(b);
                push(a);
        }
        else if (*tok == ' ')
        {
                /* do nothing */
        }
        else
        {
                SerialPrint("Unknown token: ");
                SerialPrint(tok);
                SerialPrint("\r\n");
        }

        (*buf) += 1;
}

void forth(char *buf)
{
        while (*buf)
        {
                executeToken(&buf);
        }
}
