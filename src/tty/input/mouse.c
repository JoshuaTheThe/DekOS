#include <tty/input/input.h>
#include <tty/render/render.h>
#include <drivers/math.h>

char mouse_icon = 0;
int prev_save_x = -1, prev_save_y = -1;
int cursor_saved = 0;
uint32_t cursor_backbuffer[CURSOR_WIDTH * CURSOR_HEIGHT];

char mouseGetIcon(void)
{
        return mouse_icon;
}

void mouseSetIcon(char chr)
{
        mouse_icon = chr;
}

void mouseFetch(uint32_t *mx, uint32_t *my, uint32_t *prev_mx, uint32_t *prev_my, uint8_t *buttons)
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
                        mouseRestorePixels(prev_save_x, prev_save_y);
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

                framebuffer_t framebuffer = getframebuffer();
                *mx = *mx < 0 ? 0 : (*mx >= framebuffer.dimensions[0] ? framebuffer.dimensions[0] - 1 : *mx);
                *my = *my < 0 ? 0 : (*my >= framebuffer.dimensions[1] ? framebuffer.dimensions[1] - 1 : *my);

                if (*mx != *prev_mx || *my != *prev_my)
                {
                        mouseSavePixels(*mx, *my);
                        prev_save_x = *mx;
                        prev_save_y = *my;
                        cursor_saved = 1;

                        setfont(&cursors);
                        displaychar(mouse_icon, *mx, *my, 0x00000000, 0xFFFFFFFF);
                        setfont(&font_8x8);
                        *prev_mx = *mx;
                        *prev_my = *my;
                }

                *buttons = packet.buttons;
                break;
        }
        sti();
}

void mouseSavePixels(int x, int y)
{
        framebuffer_t framebuffer = getframebuffer();
        int start_x = x;
        int start_y = y;
        int end_x = min(x + CURSOR_WIDTH, framebuffer.dimensions[0]);
        int end_y = min(y + CURSOR_HEIGHT, framebuffer.dimensions[1]);
        int actual_width = end_x - start_x;
        int actual_height = end_y - start_y;

        uint32_t *fb = framebuffer.buffer;
        int pitch_in_pixels = framebuffer.dimensions[0]; // Assuming framebuffer is width * height array

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

void mouseRestorePixels(int x, int y)
{
        framebuffer_t framebuffer = getframebuffer();
        int start_x = x;
        int start_y = y;
        int end_x = min(x + CURSOR_WIDTH, framebuffer.dimensions[0]);
        int end_y = min(y + CURSOR_HEIGHT, framebuffer.dimensions[1]);
        int actual_width = end_x - start_x;
        int actual_height = end_y - start_y;

        uint32_t *fb = framebuffer.buffer;
        int pitch_in_pixels = framebuffer.dimensions[0];

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

bool mouseIsPresent(void)
{
        ps2_write_command(PS2_CMD_ENABLE_PORT2);

        // dummy wait, should be replaced with a Wait call, when i impl that
        for (volatile int i = 0; i < 10000; i++)
                __asm volatile("nop");

        ps2_write_command(PS2_CMD_READ_CONFIG);
        if (!ps2_wait_output())
        {
                return false;
        }

        ps2_write_command(PS2_CMD_READ_CONFIG);
        uint8_t config = ps2_read_data();
        config |= MOUSE_CLK2;
        ps2_write_command(PS2_CMD_WRITE_CONFIG);
        ps2_write_data(config);

        if (!(config & MOUSE_CLK2))
        {
                return false;
        }

        ps2_write_command(PS2_CMD_TEST_PORT2);
        if (!ps2_wait_output())
        {
                return false;
        }

        uint8_t test_result = ps2_read_data();
        if (test_result != 0x00)
        {
                return false;
        }

        if (!ps2_send_device_command(2, PS2_DEV_IDENTIFY))
        {
                return false;
        }

        if (!ps2_wait_output())
        {
                return false;
        }

        uint8_t id1 = ps2_read_data();
        uint8_t id2 __attribute__((unused)) = 0x00;
        if (ps2_wait_output())
        {
                id2 = ps2_read_data();
        }

        if (id1 == MOUSE_STANDARD || id1 == MOUSE_HAS_SCROLL || id1 == MOUSE_HAS_5_BUTTONS)
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
