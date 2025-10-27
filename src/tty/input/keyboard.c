#include <tty/input/input.h>

volatile uint8_t character = 0;
volatile bool key_pressed = false;

/* magic nums oh no */
uint8_t keyboard_map[256] =
    {
        0x00, 0x7F, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0x80, 'a', 's',
        'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0x00, '\\', 'z', 'x', 'c', 'v',
        'b', 'n', 'm', ',', '.', '/', 0x81, '*', 0x81, ' ',
        0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E,
        0x8F, 0x90, '-', 0x91, '5', 0x92, '+', 0x93, 0x94, 0x95, 0x96, 0x97, '\n',
        0x81, '\\', 0x98, 0x99};

uint8_t keyboard_map_shifted[256] =
    {
        0x00, 0x7F, '!', '"', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0x80, 'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '@', '`', 0x00, '|', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', '<', '>', '?', 0x81, '*', 0x81, ' ',
        0x80, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, '/', '7',
        0x8F, 0x90, '-', 0x91, '5', 0x92, '+', 0x93, 0x94, 0x95, 0x96, 0x97, '\n',
        0x81, '\\', 0x98, 0x99};

uint8_t keyboard_get(volatile bool *hit)
{
        cli();
        static uint32_t shifted = 0;
        static uint32_t ctrl = 0;
        *hit = false;

        if (!(inb(0x64) & 0x01))
        {
                sti();
                return 0;
        }

        uint8_t status = inb(0x64);
        if (status & (1 << 5))
        {
                sti();
                return 0;
        }

        uint16_t scancode = inb(0x60);

        if (scancode == 0x2a || scancode == 0x36)
        {
                shifted = 1;
                sti();
                return 0;
        }
        else if (scancode == 0xaa || scancode == 0xb6)
        {
                shifted = 0;
                sti();
                return 0;
        }
        if (scancode == 0x1d)
        {
                ctrl = 1;
                sti();
                return 0;
        }
        else if (scancode == 0x9d)
        {
                ctrl = 0;
                sti();
                return 0;
        }
        if (scancode & 0x80)
        {
                sti();
                return 0;
        }
        if (scancode >= 128)
        {
                sti();
                return 0;
        }

        uint8_t key = shifted ? keyboard_map_shifted[scancode] : keyboard_map[scancode];

        *hit = true;
        if (ctrl)
        {
                if (key >= 'a' && key <= 'z')
                {
                        sti();
                        return key - 'a' + 1;
                }
                else if (key >= 'A' && key <= 'Z')
                {
                        sti();
                        return key - 'A' + 1;
                }
        }
        sti();
        return key;
}


bool is_keyboard_present(void)
{
        while (ps2_read_status() & PS2_STATUS_OUTPUT_FULL)
        {
                ps2_read_data();
        }

        // Send "Identify" command
        ps2_write_data(0xF2);

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

        uint8_t id1 = ps2_read_data();

        uint8_t id2 = 0x00;
        if (ps2_wait_output())
        {
                id2 = ps2_read_data();
        }

        // Check keyboard identification
        // 0xAB = Keyboard, 0x83 = Mouse (but 0xAB, 0x83 is modern keyboard)
        if (id1 == 0xAB)
        {
                return true;
        }

        return false;
}
