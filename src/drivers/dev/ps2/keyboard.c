#include <drivers/dev/ps2/ps2.h>
#include <tty/output.h>

volatile uint8_t character = 0;
volatile bool keyboardPressed = false;

uint8_t ps2_last_character(void)
{
        if (keyboardPressed)
        {
                return character;
        }
        return (uint8_t)-1;
}

bool ps2_pressed(void)
{
        return keyboardPressed;
}

uint8_t ps2_getchar(void)
{
        while (!keyboardPressed)
        {
                character = ps2_keyboard_fetch(NULL);
        }
        uint8_t x = character;
        keyboardPressed = false;
        return x;
}

/* magic nums oh no */
static uint8_t keyboard_map[256] =
    {
        0x00, '\e', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0x80, 'a', 's',
        'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0x00, '\\', 'z', 'x', 'c', 'v',
        'b', 'n', 'm', ',', '.', '/', 0x81, '*', 0x81, ' ',
        0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E,
        0x8F, 0x90, '-', 0x91, '5', 0x92, '+', 0x93, 0x94, 0x95, 0x96, 0x97, '\n',
        0x81, '\\', 0x98, 0x99};

static uint8_t keyboard_map_shifted[256] =
    {
        0x00, '\e', '!', '"', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0x80, 'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '@', '`', 0x00, '|', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', '<', '>', '?', 0x81, '*', 0x81, ' ',
        0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, '/', '7',
        0x8F, 0x90, '-', 0x91, '5', 0x92, '+', 0x93, 0x94, 0x95, 0x96, 0x97, '\n',
        0x81, '\\', 0x98, 0x99};

uint8_t ps2_keyboard_fetch(volatile bool *hit)
{
        static uint32_t shifted = 0;
        static uint32_t ctrl = 0;
        if (hit)
                *hit = false;
        keyboardPressed = false;
        uint32_t flags;
        pushf();
        cli();
        if (!(inb(0x64) & 0x01))
        {
                popf();
                return 0;
        }

        uint8_t status = inb(0x64);
        if (status & (1 << 5))
        {
                popf();
                return 0;
        }

        uint16_t scancode = inb(0x60);
        popf();

        if (scancode == 0x2a || scancode == 0x36)
        {
                shifted = 1;
                return 0;
        }
        else if (scancode == 0xaa || scancode == 0xb6)
        {
                shifted = 0;
                return 0;
        }
        if (scancode == 0x1d)
        {
                ctrl = 1;
                return 0;
        }
        else if (scancode == 0x9d)
        {
                ctrl = 0;
                return 0;
        }
        if (scancode & 0x80)
        {
                return 0;
        }
        if (scancode >= 128)
        {
                return 0;
        }

        uint8_t key = shifted ? keyboard_map_shifted[scancode] : keyboard_map[scancode];
        if (hit)
                *hit = true;
        keyboardPressed = true;
        if (ctrl)
        {
                if (key >= 'a' && key <= 'z')
                {
                        return key - 'a' + 1;
                }
                else if (key >= 'A' && key <= 'Z')
                {
                        return key - 'A' + 1;
                }
        }
        return key;
}

void ps2_initialize_keyboard(void)
{
        ps2_write_command(PS2_CMD_ENABLE_PORT1);
        for (volatile int i = 0; i < 10000; i++)
                __asm volatile("nop");

        ps2_write_data(0xFF);
        for (volatile int i = 0; i < 100000; i++)
                __asm volatile("nop");

        if (!ps2_wait_output())
                return;

        uint8_t reset_response = ps2_read_data();
        if (reset_response != 0xAA && reset_response != 0xFA)
        {
                printf("Keyboard reset failed: 0x%x\n", reset_response);
                return;
        }

        while (ps2_wait_output())
                ps2_read_data();

        ps2_write_data(0xF0);
        ps2_write_data(0x02);
        for (volatile int i = 0; i < 10000; i++)
                __asm volatile("nop");
        ps2_write_data(0xF4);
        for (volatile int i = 0; i < 10000; i++)
                __asm volatile("nop");
        ps2_write_data(0xF3);
        ps2_write_data(0x20);
        for (volatile int i = 0; i < 10000; i++)
                __asm volatile("nop");
}

bool ps2_keyboard_present(void)
{
        while (ps2_read_status() & PS2_STATUS_OUTPUT_FULL)
        {
                ps2_read_data();
        }

        ps2_write_data(0xEE);

        if (!ps2_wait_output())
        {
                return false;
        }

        uint8_t response = ps2_read_data();
        if (response != 0xEE)
        {
                return false;
        }

        ps2_write_data(0xFF);

        if (!ps2_wait_output())
        {
                return false;
        }

        uint8_t ack = ps2_read_data();
        if (ack != 0xFA)
        {
                return false;
        }

        if (!ps2_wait_output())
        {
                return false;
        }

        uint8_t reset_code = ps2_read_data();
        if (reset_code != 0xAA)
        {
                return false;
        }

        return true;
}
