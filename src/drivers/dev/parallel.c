#include <drivers/dev/parallel.h>
#include <io.h>
#include <memory/string.h>
#include <tty/output/output.h>

static parallel_port_t ports[3] = {
    {LPT1_BASE, PAR_MODE_SPP, false, 7, false, "LPT1"},
    {LPT2_BASE, PAR_MODE_SPP, false, 5, false, "LPT2"},
    {LPT3_BASE, PAR_MODE_SPP, false, -1, false, "LPT3"}};

bool parallel_detect(uint16_t base)
{
        if (base == 0)
                return false;

        uint8_t original_ctrl = inb(base + PAR_CTRL_REG);
        outb(base + PAR_CTRL_REG, 0x55);
        uint8_t read1 = inb(base + PAR_CTRL_REG);
        outb(base + PAR_CTRL_REG, 0xAA);
        uint8_t read2 = inb(base + PAR_CTRL_REG);
        outb(base + PAR_CTRL_REG, original_ctrl);
        return (read1 == 0x55 && read2 == 0xAA);
}

parallel_port_t *parallel_init(uint16_t base, parallel_mode_t mode)
{
        parallel_port_t *port = NULL;

        for (int i = 0; i < 3; i++)
        {
                if (ports[i].base_addr == base)
                {
                        port = &ports[i];
                        break;
                }
        }

        if (!port)
        {
                printf("Parallel: Port 0x%X not in known list\n", base);
                return NULL;
        }

        if (!parallel_detect(base))
        {
                printf("Parallel: Port 0x%X not detected\n", base);
                port->present = false;
                return NULL;
        }

        port->present = true;
        port->mode = mode;

        uint8_t ctrl = PAR_CTRL_SELECT | PAR_CTRL_INIT;
        outb(base + PAR_CTRL_REG, ctrl);

        if (parallel_detect_epp(port))
        {
                port->mode = PAR_MODE_EPP;
                printf("Parallel: Port 0x%X is EPP capable\n", base);
        }
        else if (parallel_detect_ecp(port))
        {
                port->mode = PAR_MODE_ECP;
                printf("Parallel: Port 0x%X is ECP capable\n", base);
        }

        printf("Parallel: %s initialized at 0x%X (mode: %d)\n",
                port->name, base, port->mode);

        return port;
}

void parallel_write_byte(parallel_port_t *port, uint8_t data)
{
        if (!port || !port->present)
                return;

        parallel_wait_busy(port, 1000);

        outb(port->base_addr + PAR_DATA_REG, data);
        uint8_t ctrl = inb(port->base_addr + PAR_CTRL_REG);
        outb(port->base_addr + PAR_CTRL_REG, ctrl & ~PAR_CTRL_STROBE);
        for (int i = 0; i < 10000; ++i);
        outb(port->base_addr + PAR_CTRL_REG, ctrl | PAR_CTRL_STROBE);
        for (int i = 0; i < 10000; ++i);
}

uint8_t parallel_read_byte(parallel_port_t *port)
{
        if (!port || !port->present)
                return 0xFF;
        if (port->mode == PAR_MODE_SPP)
        {
                printf("Parallel: Port not bidirectional\n");
                return 0xFF;
        }
        if (port->mode == PAR_MODE_PS2)
        {
                uint8_t ctrl = inb(port->base_addr + PAR_CTRL_REG);
                outb(port->base_addr + PAR_CTRL_REG, ctrl | PAR_CTRL_BIDIR);
        }

        parallel_wait_busy(port, 1000000);
        return inb(port->base_addr + PAR_DATA_REG);
}

bool parallel_wait_busy(parallel_port_t *port, uint32_t timeout_ms)
{
        if (!port || !port->present)
                return false;

        while (timeout_ms--)
        {
                uint8_t status = inb(port->base_addr + PAR_STATUS_REG);

                if (status & PAR_STAT_BUSY)
                {
                        return true;
                }
        }

        printf("Parallel: Timeout waiting for BUSY\n");
        return false;
}

bool parallel_detect_epp(parallel_port_t *port)
{
        if (!port || !port->present)
                return false;

        uint8_t original_ctrl = inb(port->base_addr + PAR_CTRL_REG);
        outb(port->base_addr + PAR_CTRL_REG, 0x34); 
        uint8_t test = inb(port->base_addr + 0x400);
        outb(port->base_addr + PAR_CTRL_REG, original_ctrl);
        return (test != 0xFF);
}

bool parallel_detect_ecp(parallel_port_t *port)
{
        if (!port || !port->present)
                return false;
        uint8_t ecr = inb(port->base_addr + 0x402);
        outb(port->base_addr + 0x402, 0x34);
        uint8_t test = inb(port->base_addr + 0x402);
        outb(port->base_addr + 0x402, ecr);

        return (test == 0x34);
}

void parallel_send_string(parallel_port_t *port, const char *str)
{
        if (!port || !port->present || !str)
                return;
        while (*str)
        {
                parallel_write_byte(port, *str++);
        }
}

void parallel_send_data(parallel_port_t *port, const uint8_t *data, size_t len)
{
        if (!port || !port->present || !data)
                return;
        for (size_t i = 0; i < len; i++)
        {
                parallel_write_byte(port, data[i]);
        }
}

void parallel_enable_irq(parallel_port_t *port, bool enable)
{
        if (!port || !port->present)
                return;
        uint8_t ctrl = inb(port->base_addr + PAR_CTRL_REG);
        if (enable)
        {
                ctrl |= PAR_CTRL_IRQEN;
                printf("Parallel: IRQ enabled on %s\n", port->name);
        }
        else
        {
                ctrl &= ~PAR_CTRL_IRQEN;
        }
        outb(port->base_addr + PAR_CTRL_REG, ctrl);
        port->irq_enabled = enable;
}

void parallel_irq_handler(parallel_port_t *port)
{
        if (!port || !port->present || !port->irq_enabled)
                return;
        uint8_t status = inb(port->base_addr + PAR_STATUS_REG);
        printf("Parallel: IRQ on %s, status=0x%02X\n", port->name, status);
}

void parallel_scan_ports(void)
{
        printf("Parallel: Scanning for ports...\n");

        int found = 0;
        for (int i = 0; i < 3; i++)
        {
                if (parallel_detect(ports[i].base_addr))
                {
                        parallel_init(ports[i].base_addr, PAR_MODE_SPP);
                        found++;
                }
        }

        printf("Parallel: Found %d ports\n", found);
}

void init_parallel_ports()
{
        parallel_scan_ports();
        parallel_port_t *lpt1 = parallel_init(LPT1_BASE, PAR_MODE_SPP);

        if (lpt1)
        {
                parallel_send_string(lpt1, "--DekOS--\r\nParallel port initialized\r\n");
        }
}
