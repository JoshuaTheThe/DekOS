#ifndef PARALLEL_H
#define PARALLEL_H

#include <stdint.h>
#include <stdbool.h>
#include <utils.h>

#define LPT1_BASE 0x378
#define LPT2_BASE 0x278
#define LPT3_BASE 0x3BC

#define PAR_DATA_REG 0x00   // Data register (R/W)
#define PAR_STATUS_REG 0x01 // Status register (R)
#define PAR_CTRL_REG 0x02   // Control register (R/W)

#define PAR_STAT_BUSY (1 << 7)   // 0=Busy (inverted)
#define PAR_STAT_ACK (1 << 6)    // Acknowledge
#define PAR_STAT_PAPER (1 << 5)  // Paper out
#define PAR_STAT_SELECT (1 << 4) // Select printer
#define PAR_STAT_ERROR (1 << 3)  // Error (0=Error, inverted)

#define PAR_CTRL_STROBE (1 << 0) // Strobe (0=Active)
#define PAR_CTRL_AUTOFD (1 << 1) // Auto feed (0=Active)
#define PAR_CTRL_INIT (1 << 2)   // Initialize printer (0=Reset)
#define PAR_CTRL_SELECT (1 << 3) // Select printer (1=Select)
#define PAR_CTRL_IRQEN (1 << 4)  // Enable interrupt
#define PAR_CTRL_BIDIR (1 << 5)  // 1=Bi-directional

typedef enum
{
        PAR_MODE_SPP,
        PAR_MODE_PS2,
        PAR_MODE_EPP,
        PAR_MODE_ECP
} parallel_mode_t;

typedef struct
{
        uint16_t base_addr;
        parallel_mode_t mode;
        bool irq_enabled;
        uint8_t irq_line;
        bool present;
        const char *name;
} parallel_port_t;

bool parallel_detect(uint16_t base);
parallel_port_t *parallel_init(uint16_t base, parallel_mode_t mode);
void parallel_write_byte(parallel_port_t *port, uint8_t data);
uint8_t parallel_read_byte(parallel_port_t *port);
bool parallel_wait_busy(parallel_port_t *port, uint32_t timeout_ms);
void parallel_set_mode(parallel_port_t *port, parallel_mode_t mode);
void parallel_enable_irq(parallel_port_t *port, bool enable);
void parallel_send_string(parallel_port_t *port, const char *str);
void parallel_send_data(parallel_port_t *port, const uint8_t *data, size_t len);
bool parallel_detect_epp(parallel_port_t *port);
bool parallel_detect_ecp(parallel_port_t *port);
void parallel_epp_write(parallel_port_t *port, uint8_t addr, uint32_t data);
uint32_t parallel_epp_read(parallel_port_t *port, uint8_t addr);
void init_parallel_ports();

#endif
