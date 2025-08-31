#include <input.h>

int cursor_saved = 0;
int prev_save_x = -1, prev_save_y = -1;
uint32_t cursor_backbuffer[CURSOR_WIDTH * CURSOR_HEIGHT];

char mouse_icon = 0;

volatile uint8_t character = 0;
volatile bool key_pressed = false;

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

int gets(char *b, int max) /* return length */
{
        if (!b) return 0;
        int i, ch;
        for (i=0; i<max; ++i)
        {
                ch = getchar();
                k_putch(ch);
                k_display();
                if (ch == '\b')
                {
                        i -= 2;
                        b[i+1] = '\0';
                        continue;
                }
                else if (ch == '\n' || ch == '\r')
                {
                        break;
                }
                b[i] = ch;
        }
        return i;
}

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

void mouse_get(int *mx, int *my, int *prev_mx, int *prev_my, uint8_t *buttons)
{
        cli();
        ps2_mouse_packet_t packet;
        static float sensitivity = 0.0001f;
        static int acceleration_threshold = 5;
        static float acceleration_factor = 1.0f;
        static float divisor = 100;
        static float smoothing_factor = 0.5f;
        static float dx_smooth = 0, dy_smooth = 0;

        while (ps2_read_mouse_packet(&packet))
        {
                // Process the packet
                if (cursor_saved)
                {
                        restore_pixels(prev_save_x, prev_save_y);
                        cursor_saved = 0;
                }

                float raw_dx = (float)packet.x_movement / divisor;
                float raw_dy = (float)packet.y_movement / divisor;
                dx_smooth = smoothing_factor * raw_dx + (1 - smoothing_factor) * dx_smooth;
                dy_smooth = smoothing_factor * raw_dy + (1 - smoothing_factor) * dy_smooth;
                float movement_magnitude = sqrtf(dx_smooth * dx_smooth + dy_smooth * dy_smooth);
                float accel = 0.5f;
                if (movement_magnitude > acceleration_threshold)
                {
                        float accel_amount = (movement_magnitude - acceleration_threshold) * 0.08f;
                        accel = 1.0f + acceleration_factor * (1.0f - expf(-accel_amount));
                        accel = fminf(accel, 4.0f);
                }

                float final_dx = dx_smooth * sensitivity * accel;
                float final_dy = dy_smooth * sensitivity * accel;

                static float subpixel_x = 0, subpixel_y = 0;
                subpixel_x += final_dx;
                subpixel_y += final_dy;

                int pixel_dx = (int)subpixel_x;
                int pixel_dy = (int)subpixel_y;
                subpixel_x -= pixel_dx;
                subpixel_y -= pixel_dy;

                *mx += pixel_dx;
                *my -= pixel_dy;

                *mx = *mx < 0 ? 0 : (*mx >= framebuffer_width ? framebuffer_width - 1 : *mx);
                *my = *my < 0 ? 0 : (*my >= framebuffer_height ? framebuffer_height - 1 : *my);

                if (*mx != *prev_mx || *my != *prev_my)
                {
                        save_pixels(*mx, *my);
                        prev_save_x = *mx;
                        prev_save_y = *my;
                        cursor_saved = 1;

                        set_active_font(&cursors);
                        put_character(mouse_icon, *mx, *my, 0x00000000, 0xFFFFFFFF);
                        set_active_font(&font_8x8);
                        *prev_mx = *mx;
                        *prev_my = *my;
                }

                *buttons = packet.buttons;
                break;
        }
        sti();
}

void save_pixels(int x, int y)
{
        int start_x = x;
        int start_y = y;
        int end_x = min(x + CURSOR_WIDTH, framebuffer_width);
        int end_y = min(y + CURSOR_HEIGHT, framebuffer_height);
        int actual_width = end_x - start_x;
        int actual_height = end_y - start_y;

        uint32_t *fb = framebuffer;
        int pitch_in_pixels = framebuffer_width; // Assuming framebuffer is width * height array

        for (int j = 0; j < actual_height; j++)
        {
                for (int i = 0; i < actual_width; i++)
                {
                        int fb_index = (start_y + j) * pitch_in_pixels + (start_x + i);
                        int bb_index = j * CURSOR_WIDTH + i;
                        cursor_backbuffer[bb_index] = fb[fb_index];
                }
        }
}

void restore_pixels(int x, int y)
{
        int start_x = x;
        int start_y = y;
        int end_x = min(x + CURSOR_WIDTH, framebuffer_width);
        int end_y = min(y + CURSOR_HEIGHT, framebuffer_height);
        int actual_width = end_x - start_x;
        int actual_height = end_y - start_y;

        uint32_t *fb = framebuffer;
        int pitch_in_pixels = framebuffer_width;

        for (int j = 0; j < actual_height; j++)
        {
                for (int i = 0; i < actual_width; i++)
                {
                        int fb_index = (start_y + j) * pitch_in_pixels + (start_x + i);
                        int bb_index = j * CURSOR_WIDTH + i;
                        fb[fb_index] = cursor_backbuffer[bb_index];
                }
        }
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

bool is_mouse_present(void)
{
        // Enable the second PS/2 port (mouse port)
        ps2_write_command(PS2_CMD_ENABLE_PORT2);

        // Wait a bit for the port to enable
        for (volatile int i = 0; i < 10000; i++)
                __asm volatile("nop");

        // Read the configuration uint8_t
        ps2_write_command(PS2_CMD_READ_CONFIG);
        if (!ps2_wait_output())
        {
                return false;
        }

        ps2_write_command(PS2_CMD_READ_CONFIG);
        uint8_t config = ps2_read_data();
        config |= (1 << 5); // Enable port 2 clock
        ps2_write_command(PS2_CMD_WRITE_CONFIG);
        ps2_write_data(config);

        // Check if the second port is actually enabled
        if (!(config & (1 << 5)))
        {
                return false; // Second port clock is disabled
        }

        // Test the second port
        ps2_write_command(PS2_CMD_TEST_PORT2);
        if (!ps2_wait_output())
        {
                return false;
        }

        uint8_t test_result = ps2_read_data();
        if (test_result != 0x00)
        {
                return false; // Port test failed
        }

        // Try to identify the mouse
        if (!ps2_send_device_command(2, PS2_DEV_IDENTIFY))
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

        // Check for mouse identification
        // Standard mouse returns 0x00
        // Scroll wheel mouse returns 0x03
        // 5-button mouse returns 0x04
        if (id1 == 0x00 || id1 == 0x03 || id1 == 0x04)
        {
                return true;
        }

        return false;
}

void ps2_enable_mouse_interrupts(void)
{
        ps2_write_command(PS2_CMD_READ_CONFIG);
        if (!ps2_wait_output())
                return;
        uint8_t config = ps2_read_data();
        config |= (1 << 1) | (1 << 5);
        ps2_write_command(PS2_CMD_WRITE_CONFIG);
        ps2_write_data(config);
}

void ps2_initialize_mouse(void)
{
        ps2_write_command(PS2_CMD_ENABLE_PORT2);
        for (volatile int i = 0; i < 10000; i++)
                __asm volatile("nop");
        // ps2_enable_mouse_interrupts();
        ps2_send_device_command(2, PS2_DEV_ENABLE_SCAN);
        ps2_send_device_command(2, 0xF3); // Set sample rate
        ps2_send_device_command(2, 100);  // 100 samples/sec
        ps2_send_device_command(2, 0xF4);
}

bool ps2_read_mouse_packet(ps2_mouse_packet_t *packet)
{
        static uint8_t packet_data[4];
        static uint8_t packet_index = 0;
        static uint8_t packet_size = 3;

        if (!(ps2_read_status() & PS2_STATUS_OUTPUT_FULL))
                return false;

        uint8_t status = ps2_read_status();
        if (!(status & (1 << 5)))
        {
                return false;
        }

        uint8_t data = ps2_read_data();

        if (packet_index == 0)
        {
                if (!(data & (1 << 3)))
                {
                        packet_index = 0;
                        return false;
                }
        }

        packet_data[packet_index++] = data;

        if (packet_index >= packet_size)
        {
                packet_index = 0;

                packet->flags = packet_data[0];
                packet->x_movement = packet_data[1];
                packet->y_movement = packet_data[2];

                if (packet_data[0] & (1 << 6))
                        packet->x_movement = 0;
                if (packet_data[0] & (1 << 7))
                        packet->y_movement = 0;

                if (packet_data[0] & (1 << 4))
                        packet->x_movement -= 256;
                if (packet_data[0] & (1 << 5))
                        packet->y_movement -= 256;

                packet->buttons = 0;
                if (packet_data[0] & (1 << 0))
                        packet->buttons |= MOUSE_LEFT_BUTTON;
                if (packet_data[0] & (1 << 1))
                        packet->buttons |= MOUSE_RIGHT_BUTTON;
                if (packet_data[0] & (1 << 2))
                        packet->buttons |= MOUSE_MIDDLE_BUTTON;

                packet->z_movement = 0;

                return true;
        }

        return false;
}
