#include <tty/input/input.h>
#include <tty/render/render.h>
#include <drivers/math.h>

extern KRNLRES *fbRes;

unsigned char mouse_icon = 0;
int prev_save_x = -1, prev_save_y = -1;
static uint32_t cursor_backbuffer[CURSOR_WIDTH * CURSOR_HEIGHT];
static bool cursor_saved = false;

unsigned char mouseGetIcon(void)
{
        return mouse_icon;
}

void mouseSetIcon(unsigned char chr)
{
        mouse_icon = chr;
}

void mouseFetch(int *mx, int *my, int *prev_mx, int *prev_my, uint8_t *buttons)
{
        int iDim[3];
        if (!fbRes || !fbRes->Region.ptr)
        {
                return;
        }
        RenderGetDim(iDim);

        ps2_mouse_packet_t packet;

        while (ps2_read_mouse_packet(&packet))
        {
                if (cursor_saved)
                {
                        mouseRestorePixels(prev_save_x, prev_save_y);
                        cursor_saved = 0;
                }

                int delta_x = (int8_t)packet.x_movement;
                int delta_y = (int8_t)packet.y_movement;

                *mx += delta_x;
                *my -= delta_y;

                *mx = *mx < 0 ? 0 : (*mx >= (int)iDim[0] ? (int)iDim[0] - 1 : *mx);
                *my = *my < 0 ? 0 : (*my >= (int)iDim[1] ? (int)iDim[1] - 1 : *my);

                mouseSavePixels(*mx, *my);
                prev_save_x = *mx;
                prev_save_y = *my;
                cursor_saved = 1;

                RenderSetFont(&cursors);
                RenderChar(mouse_icon, *mx, *my, 0x00000000, 0xFFFFFFFF);
                RenderSetFont(&font_8x8);
                *prev_mx = *mx;
                *prev_my = *my;

                *buttons = packet.buttons;
                break;
        }
}

void mouseSavePixels(uint32_t x, uint32_t y)
{
        int iDim[3];
        if (!fbRes || !fbRes->Region.ptr)
        {
                return;
        }

        RenderGetDim(iDim);
        uint32_t start_x = x;
        uint32_t start_y = y;
        uint32_t end_x = minu(x + CURSOR_WIDTH, iDim[0]);
        uint32_t end_y = minu(y + CURSOR_HEIGHT, iDim[1]);
        size_t actual_width = end_x - start_x;
        size_t actual_height = end_y - start_y;

        uint32_t *fb = fbRes->Region.ptr;
        size_t pitch_in_pixels = iDim[0];

        for (size_t j = 0; j < actual_height; j++)
        {
                for (size_t i = 0; i < actual_width; i++)
                {
                        size_t fb_index = (start_y + j) * pitch_in_pixels + (start_x + i);
                        size_t bb_index = j * CURSOR_WIDTH + i;
                        cursor_backbuffer[bb_index] = fb[fb_index];
                }
        }
}

void mouseRestorePixels(uint32_t x, uint32_t y)
{
        int iDim[3];
        if (!fbRes || !fbRes->Region.ptr)
        {
                return;
        }

        RenderGetDim(iDim);
        uint32_t start_x = x;
        uint32_t start_y = y;
        uint32_t end_x = minu(x + CURSOR_WIDTH, iDim[0]);
        uint32_t end_y = minu(y + CURSOR_HEIGHT, iDim[1]);
        size_t actual_width = end_x - start_x;
        size_t actual_height = end_y - start_y;

        uint32_t *fb = fbRes->Region.ptr;
        size_t pitch_in_pixels = iDim[0];

        for (size_t j = 0; j < actual_height; j++)
        {
                for (size_t i = 0; i < actual_width; i++)
                {
                        size_t fb_index = (start_y + j) * pitch_in_pixels + (start_x + i);
                        size_t bb_index = j * CURSOR_WIDTH + i;
                        fb[fb_index] = cursor_backbuffer[bb_index];
                }
        }
}

bool mouseIsPresent(void)
{
        ps2_write_command(PS2_CMD_ENABLE_PORT2);

        // dummy wait, should be replaced with a Wait call, when i impl that
        for (volatile size_t i = 0; i < 10000; i++)
                __asm volatile("nop");

        ps2_write_command(PS2_CMD_READ_CONFIG);
        if (!ps2_wait_output())
        {
                return false;
        }

        ps2_write_command(PS2_CMD_READ_CONFIG);
        uint8_t config = ps2_read_data();
        config |= (1 << 5);
        ps2_write_command(PS2_CMD_WRITE_CONFIG);
        ps2_write_data(config);

        if (!(config & (1 << 5)))
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

        // Reset mouse
        ps2_send_device_command(2, 0xFF);
        for (volatile int i = 0; i < 100000; i++)
                __asm volatile("nop");

        // Enable data reporting
        ps2_send_device_command(2, 0xF4);
        for (volatile int i = 0; i < 10000; i++)
                __asm volatile("nop");

        // Set sample rate to something reasonable
        ps2_send_device_command(2, 0xF3);
        ps2_send_device_command(2, 100);
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
                        packet->x_movement |= 0xFFFFFF00;
                if (packet_data[0] & (1 << 5))
                        packet->y_movement |= 0xFFFFFF00;

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
