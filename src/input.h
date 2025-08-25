#ifndef PS2_H
#define PS2_H

#include <stdint.h>
#include <stdbool.h>
#include <text.h>
#include <fonts.h>

// PS/2 Port Definitions
#define PS2_DATA_PORT 0x60
#define PS2_STATUS_PORT 0x64
#define PS2_COMMAND_PORT 0x64

// PS/2 Status Register Bits
#define PS2_STATUS_OUTPUT_FULL (1 << 0)
#define PS2_STATUS_INPUT_FULL (1 << 1)
#define PS2_STATUS_SYSTEM_FLAG (1 << 2)
#define PS2_STATUS_COMMAND_DATA (1 << 3)
#define PS2_STATUS_TIMEOUT_ERROR (1 << 6)
#define PS2_STATUS_PARITY_ERROR (1 << 7)

// PS/2 Controller Commands
#define PS2_CMD_READ_CONFIG 0x20
#define PS2_CMD_WRITE_CONFIG 0x60
#define PS2_CMD_DISABLE_PORT2 0xA7
#define PS2_CMD_ENABLE_PORT2 0xA8
#define PS2_CMD_TEST_PORT2 0xA9
#define PS2_CMD_TEST_CONTROLLER 0xAA
#define PS2_CMD_TEST_PORT1 0xAB
#define PS2_CMD_DISABLE_PORT1 0xAD
#define PS2_CMD_ENABLE_PORT1 0xAE
#define PS2_CMD_WRITE_PORT2 0xD4

// PS/2 Device Commands
#define PS2_DEV_IDENTIFY 0xF2
#define PS2_DEV_ENABLE_SCAN 0xF4
#define PS2_DEV_DISABLE_SCAN 0xF5
#define PS2_DEV_RESET 0xFF

// PS/2 Responses
#define PS2_RESP_ACK 0xFA
#define PS2_RESP_SELF_TEST_PASS 0xAA
#define PS2_RESP_ECHO 0xEE
#define PS2_RESP_RESEND 0xFE

// Mouse packet structure
typedef struct
{
        uint8_t flags;
        int16_t x_movement;
        int16_t y_movement;
        int8_t z_movement; // For scroll wheel mice
        uint8_t buttons;
} ps2_mouse_packet_t;

// Mouse button flags
#define MOUSE_LEFT_BUTTON (1 << 0)
#define MOUSE_RIGHT_BUTTON (1 << 1)
#define MOUSE_MIDDLE_BUTTON (1 << 2)
#define CURSOR_WIDTH 8
#define CURSOR_HEIGHT 10

uint8_t ps2_read_status(void);
uint8_t ps2_read_data(void);
void ps2_write_data(uint8_t data);
void ps2_write_command(uint8_t command);
bool ps2_wait_output(void);
bool ps2_wait_input(void);
bool is_keyboard_present(void);
bool is_mouse_present(void);
void ps2_initialize_mouse(void);
bool ps2_read_mouse_packet(ps2_mouse_packet_t *packet);
void mouse_get(int *mx, int *my, int *prev_mx, int *prev_my, uint8_t *buttons);
void save_pixels(int x, int y);
void restore_pixels(int x, int y);
int min(int a, int b);

extern char mouse_icon;
extern int prev_save_x, prev_save_y;

#endif
