#include <drivers/dev/storage/disk.h>
#include <tty/output/output.h>

__attribute__((deprecated))
static void ataWait(const uint16_t p)
{
        inb(p + CONTROL + ALTERNATE_STATUS);
        inb(p + CONTROL + ALTERNATE_STATUS);
        inb(p + CONTROL + ALTERNATE_STATUS);
        inb(p + CONTROL + ALTERNATE_STATUS);
}

__attribute__((deprecated))
int cdRead(uint16_t port, bool slave, uint32_t lba, uint32_t sectors, uint16_t *buffer)
{
        volatile uint8_t read_cmd[12] = {0xA8, 0,
                                         (lba >> 0x18) & 0xFF, (lba >> 0x10) & 0xFF, (lba >> 0x08) & 0xFF,
                                         (lba >> 0x00) & 0xFF,
                                         (sectors >> 0x18) & 0xFF, (sectors >> 0x10) & 0xFF, (sectors >> 0x08) & 0xFF,
                                         (sectors >> 0x00) & 0xFF,
                                         0, 0};

        outb(port + DRIVE_SELECT, 0xA0 & (slave << 4));
        ataWait(port);
        outb(port + ERROR_R, 0x00);
        outb(port + LBA_MID, 2048 & 0xFF);
        outb(port + LBA_HIGH, 2048 >> 8);
        outb(port + COMMAND_REGISTER, 0xA0);
        ataWait(port);

        while (1)
        {
                uint8_t status = inb(port + COMMAND_REGISTER);
                if ((status & 0x01) == 1)
                        return 1;
                if (!(status & 0x80) && (status & 0x08))
                        break;
                ataWait(port);
        }

        outsw(port + DATA, (void*)read_cmd, 6);

        for (uint32_t i = 0; i < sectors; i++)
        {
                while (1)
                {
                        uint8_t status = inb(port + COMMAND_REGISTER);
                        if (status & 0x01)
                                return 1;
                        if (!(status & 0x80) && (status & 0x08))
                                break;
                }

                int size = inb(port + LBA_HIGH) << 8 | inb(port + LBA_MID); 
                insw(port + DATA, ((uint8_t *)buffer + i * 0x800), (size_t)size / 2);
        }

        return 0;
}

__attribute__((deprecated))
int cdDetect(uint16_t port, bool slave)
{
        outb(port + DRIVE_SELECT, 0xA0 & (slave << 4));
        ataWait(port);

        outb(port + COMMAND_REGISTER, 0xA1);

        uint32_t timeout = 100000;
        while (timeout--)
        {
                uint8_t status = inb(port + STATUS);

                if (status & 0x01)
                {
                        return 0;
                }
                if (status & 0x08)
                {
                        break;
                }
                if (timeout == 0)
                {
                        return 0;
                }
                ataWait(port);
        }

        uint16_t identify_data[256];
        insw(port + DATA, identify_data, 256);

        if (!(identify_data[0] & (1 << 8)))
        {
                return 0;
        }

        uint8_t packet_type = (identify_data[0] >> 8) & 0x1F;
        return (packet_type == 0x05);
}

__attribute__((deprecated))
int cdMediaPresent(uint16_t port, bool slave)
{
        volatile uint8_t test_unit_ready[12] = {0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        outb(port + DRIVE_SELECT, 0xA0 & (slave << 4));
        ataWait(port);

        outb(port + ERROR_R, 0x00);
        outb(port + LBA_MID, 0);
        outb(port + LBA_HIGH, 0);
        outb(port + COMMAND_REGISTER, 0xA0);
        ataWait(port);

        uint32_t timeout = 100000;
        while (timeout--)
        {
                uint8_t status = inb(port + STATUS);
                if (status & 0x01)
                {
                        uint8_t error = inb(port + ERROR_R);
                        if (error & 0x04)
                        {
                                return 0;
                        }
                        return 0;
                }
                if (!(status & 0x80) && (status & 0x08))
                {
                        break;
                }
                if (timeout == 0)
                {
                        return 0;
                }
                ataWait(port);
        }

        outsw(port + DATA, (uint16_t *)test_unit_ready, 6);

        timeout = 100000;
        while (timeout--)
        {
                uint8_t status = inb(port + STATUS);
                if (status & 0x01)
                {
                        return 0;
                }
                if (!(status & 0x80))
                {
                        return 1;
                }
                if (timeout == 0)
                {
                        return 0;
                }
                ataWait(port);
        }

        return 0;
}

__attribute__((deprecated))
int cdDetectMedia(uint16_t port, bool slave)
{
        printf("Checking for CD-ROM at port %d, slave: %d, ", port, slave);

        if (!cdDetect(port, slave))
        {
                printf("Could not find\n");
                return 0;
        }

        if (cdMediaPresent(port, slave))
        {
                printf("CD media is present\n");
                return 2;
        }
        else
        {
                printf("CD media not present\n");
                return 1;
        }
}

__attribute__((deprecated))
void cdInit(int *port, bool *slave)
{
        int primary_status = cdDetectMedia(0x1F0, false);
        bool primary_is_slave = false;
        if (primary_status == 0)
        {
                primary_status = cdDetectMedia(0x1F0, true);
                primary_is_slave = true;
        }

        int secondary_status = cdDetectMedia(0x170, false);
        bool secondary_is_slave = false;
        if (secondary_status == 0)
        {
                secondary_status = cdDetectMedia(0x170, true);
                secondary_is_slave = true;
        }

        if (primary_status == 2)
        {
                *port = 0x1F0;
                *slave = primary_is_slave;
        }
        else if (secondary_status == 2)
        {
                *port = 0x170;
                *slave = secondary_is_slave;
        }
        else
        {
                *port = 0;
                *slave = false;
        }

        if (*port == 0 && *slave == 0)
        {
                printf("Failed to find CD-ROM\n");
                return;
        }

        printf("Port: %d; IsSlave: %d\n", *port, *slave);
}
