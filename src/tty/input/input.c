#include <tty/input/input.h>

char getchar(void)
{
        key_pressed = false;
        while (!key_pressed)
        {
                character = keyboard_get(&key_pressed);
        }
        char x = character;
        key_pressed = false;
        return x;
}

int gets(char *b, int max)
{
        if (!b || max <= 0)
                return 0;

        int i = 0, ch;

        while (i < max - 1)
        {
                ch = getchar();

                if (ch == '\b' || ch == 127)
                {
                        if (i > 0)
                        {
                                i--;
                                putchar('\b');
                                putchar(' ');
                                putchar('\b');
                                display();
                                b[i] = '\0';
                        }
                }
                else if (ch == '\n' || ch == '\r')
                {
                        putchar(ch);
                        display();
                        break;
                }
                else if (ch >= 32 && ch <= 126)
                {
                        putchar(ch);
                        display();
                        b[i] = ch;
                        i++;
                }
        }

        b[i] = '\0';
        return i;
}

uint8_t ps2_read_status(void)
{
        uint8_t status;
        __asm volatile("inb %1, %0" : "=a"(status) : "Nd"(PS2_STATUS_PORT));
        return status;
}

uint8_t ps2_read_data(void)
{
        while (!(ps2_read_status() & PS2_STATUS_OUTPUT_FULL))
        {
                /* timeout needed, will do when we are using pit */
        }

        uint8_t data;
        __asm volatile("inb %1, %0" : "=a"(data) : "Nd"(PS2_DATA_PORT));
        return data;
}

void ps2_write_data(uint8_t data)
{
        while (ps2_read_status() & PS2_STATUS_INPUT_FULL)
        {
                /* timeout needed, will do when we are using pit */
        }

        __asm volatile("outb %0, %1" : : "a"(data), "Nd"(PS2_DATA_PORT));
}

void ps2_write_command(uint8_t command)
{
        while (ps2_read_status() & PS2_STATUS_INPUT_FULL)
        {
                /* timeout needed, will do when we are using pit */
        }

        __asm volatile("outb %0, %1" : : "a"(command), "Nd"(PS2_COMMAND_PORT));
}

bool ps2_wait_output(void)
{
        /* timeout needed, will do when we are using pit */
        for (int i = 0; i < 10000; i++)
        {
                if (ps2_read_status() & PS2_STATUS_OUTPUT_FULL)
                {
                        return true;
                }
        }
        return false;
}

bool ps2_wait_input(void)
{
        /* timeout needed, will do when we are using pit */
        for (int i = 0; i < 10000; i++)
        {
                if (!(ps2_read_status() & PS2_STATUS_INPUT_FULL))
                {
                        return true;
                }
        }
        return false;
}

bool ps2_send_device_command(uint8_t port, uint8_t command)
{
        if (port == 2)
        {
                ps2_write_command(PS2_CMD_WRITE_PORT2);
        }

        ps2_write_data(command);

        if (!ps2_wait_output())
        {
                return false;
        }

        uint8_t response = ps2_read_data();
        return (response == PS2_RESP_ACK);
}
