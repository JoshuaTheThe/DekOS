#include <drivers/dev/storage/ide.h>

IDEDriver_t IDEState = {0};
uint8_t package[2048], atapi_packet[2048];

void IDEWaitIRQ(void)
{
        while (!IDEState.Invoked)
                ;
        IDEState.Invoked = 0;
}

void __attribute__((interrupt)) IDEIrq(void *x)
{
        IDEState.Invoked = 1;
        outb(0xA0, 0x20);
        outb(0x20, 0x20);
}

void IDEWrite(uint8_t channel, uint8_t reg, uint8_t data)
{
        if (reg > 0x07 && reg < 0x0C)
                IDEWrite(channel, ATA_REG_CONTROL, 0x80 | IDEState.Channels[channel].nIEN);
        if (reg < 0x08)
                outb(IDEState.Channels[channel].base + reg - 0x00, data);
        else if (reg < 0x0C)
                outb(IDEState.Channels[channel].base + reg - 0x06, data);
        else if (reg < 0x0E)
                outb(IDEState.Channels[channel].ctrl + reg - 0x0A, data);
        else if (reg < 0x16)
                outb(IDEState.Channels[channel].bmide + reg - 0x0E, data);
        if (reg > 0x07 && reg < 0x0C)
                IDEWrite(channel, ATA_REG_CONTROL, IDEState.Channels[channel].nIEN);
}

uint8_t IDERead(uint8_t channel, uint8_t reg)
{
        uint8_t result = 0xFF;
        if (reg > 0x07 && reg < 0x0C)
                IDEWrite(channel, ATA_REG_CONTROL, 0x80 | IDEState.Channels[channel].nIEN);
        if (reg < 0x08)
                result = inb(IDEState.Channels[channel].base + reg - 0x00);
        else if (reg < 0x0C)
                result = inb(IDEState.Channels[channel].base + reg - 0x06);
        else if (reg < 0x0E)
                result = inb(IDEState.Channels[channel].ctrl + reg - 0x0A);
        else if (reg < 0x16)
                result = inb(IDEState.Channels[channel].bmide + reg - 0x0E);
        if (reg > 0x07 && reg < 0x0C)
                IDEWrite(channel, ATA_REG_CONTROL, IDEState.Channels[channel].nIEN);
        return result;
}

void IDEReadBuffer(uint8_t channel, uint8_t reg, uint64_t buffer, uint64_t quads)
{
        if (reg > 0x07 && reg < 0x0C)
                IDEWrite(channel, ATA_REG_CONTROL, 0x80 | IDEState.Channels[channel].nIEN);
        if (reg < 0x08)
                insl(IDEState.Channels[channel].base + reg - 0x00, (void *)buffer, quads);
        else if (reg < 0x0C)
                insl(IDEState.Channels[channel].base + reg - 0x06, (void *)buffer, quads);
        else if (reg < 0x0E)
                insl(IDEState.Channels[channel].ctrl + reg - 0x0A, (void *)buffer, quads);
        else if (reg < 0x16)
                insl(IDEState.Channels[channel].bmide + reg - 0x0E, (void *)buffer, quads);
        if (reg > 0x07 && reg < 0x0C)
                IDEWrite(channel, ATA_REG_CONTROL, IDEState.Channels[channel].nIEN);
}

uint8_t IDEPrintErr(uint32_t drive, uint8_t err)
{
        if (err == 0)
                return err;

        printf("IDE:");
        if (err == 1)
        {
                printf("- Device Fault\n     ");
                err = 19;
        }
        else if (err == 2)
        {
                uint8_t st = IDERead(IDEState.IDEDev[drive].Channel, ATA_REG_ERROR);
                if (st & ATA_ER_AMNF)
                {
                        printf("- No Address Mark Found\n     ");
                        err = 7;
                }
                if (st & ATA_ER_TK0NF)
                {
                        printf("- No Media or Media Error\n     ");
                        err = 3;
                }
                if (st & ATA_ER_ABRT)
                {
                        printf("- Command Aborted\n     ");
                        err = 20;
                }
                if (st & ATA_ER_MCR)
                {
                        printf("- No Media or Media Error\n     ");
                        err = 3;
                }
                if (st & ATA_ER_IDNF)
                {
                        printf("- ID mark not Found\n     ");
                        err = 21;
                }
                if (st & ATA_ER_MC)
                {
                        printf("- No Media or Media Error\n     ");
                        err = 3;
                }
                if (st & ATA_ER_UNC)
                {
                        printf("- Uncorrectable Data Error\n     ");
                        err = 22;
                }
                if (st & ATA_ER_BBK)
                {
                        printf("- Bad Sectors\n     ");
                        err = 13;
                }
        }
        else if (err == 3)
        {
                printf("- Reads Nothing\n     ");
                err = 23;
        }
        else if (err == 4)
        {
                printf("- Write Protected\n     ");
                err = 8;
        }
        printf("- [%s %s] %s\n",
               (const char *[]){"Primary", "Secondary"}[IDEState.IDEDev[drive].Channel],
               (const char *[]){"Master", "Slave"}[IDEState.IDEDev[drive].Drive],
               IDEState.IDEDev[drive].Model);

        return err;
}

uint8_t IDEPolling(uint8_t channel, uint32_t advanced_check)
{

        for (int i = 0; i < 4; i++)
                IDERead(channel, ATA_REG_ALTSTATUS);

        while (IDERead(channel, ATA_REG_STATUS) & ATA_SR_BSY)
                ;

        if (advanced_check)
        {
                uint8_t state = IDERead(channel, ATA_REG_STATUS);
                if (state & ATA_SR_ERR)
                        return 2;
                if (state & ATA_SR_DF)
                        return 1;
                if ((state & ATA_SR_DRQ) == 0)
                        return 3;
        }

        return 0;
}

void IDEInitialise(void)
{
        if (!IDEState.Dev)
                return;
        int i, j, k, count = 0;
        IDEState.Channels[ATA_PRIMARY].base = (IDEState.Dev->bar[0] & 0xFFFFFFFC) + 0x1F0 * (!IDEState.Dev->bar[0]);
        IDEState.Channels[ATA_PRIMARY].ctrl = (IDEState.Dev->bar[1] & 0xFFFFFFFC) + 0x3F6 * (!IDEState.Dev->bar[1]);
        IDEState.Channels[ATA_SECONDARY].base = (IDEState.Dev->bar[2] & 0xFFFFFFFC) + 0x170 * (!IDEState.Dev->bar[2]);
        IDEState.Channels[ATA_SECONDARY].ctrl = (IDEState.Dev->bar[3] & 0xFFFFFFFC) + 0x376 * (!IDEState.Dev->bar[3]);
        IDEState.Channels[ATA_PRIMARY].bmide = (IDEState.Dev->bar[4] & 0xFFFFFFFC) + 0;   // Bus Master IDE
        IDEState.Channels[ATA_SECONDARY].bmide = (IDEState.Dev->bar[4] & 0xFFFFFFFC) + 8; // Bus Master IDE
        SerialPrint(" [INFO] IDE Channels Set\r\n");
        IDEWrite(ATA_PRIMARY, ATA_REG_CONTROL, 0x02);
        IDEWrite(ATA_SECONDARY, ATA_REG_CONTROL, 0x02);
        SerialPrint(" [INFO] IDE IRQs Disabled\r\n");
        for (i = 0; i < 2; i++)
                for (j = 0; j < 2; j++)
                {

                        uint8_t err = 0, type = IDE_ATA, status;
                        IDEState.IDEDev[count].Reserved = 0;

                        IDEWrite(i, ATA_REG_HDDEVSEL, 0xA0 | (j << 4));
                        pitDelay(4);
                        IDEWrite(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
                        pitDelay(4);
                        if (IDERead(i, ATA_REG_STATUS) == 0)
                                continue;

                        while (1)
                        {
                                status = IDERead(i, ATA_REG_STATUS);
                                if ((status & ATA_SR_ERR))
                                {
                                        err = 1;
                                        break;
                                }
                                if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ))
                                        break;
                        }

                        if (err != 0)
                        {
                                uint8_t cl = IDERead(i, ATA_REG_LBA1);
                                uint8_t ch = IDERead(i, ATA_REG_LBA2);

                                if (cl == 0x14 && ch == 0xEB)
                                        type = IDE_ATAPI;
                                else if (cl == 0x69 && ch == 0x96)
                                        type = IDE_ATAPI;
                                else
                                        continue;

                                IDEWrite(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
                                pitDelay(4);
                        }

                        IDEReadBuffer(i, ATA_REG_DATA, (uint64_t)IDEState.Buff, 128);

                        IDEState.IDEDev[count].Reserved = 1;
                        IDEState.IDEDev[count].Type = type;
                        IDEState.IDEDev[count].Channel = i;
                        IDEState.IDEDev[count].Drive = j;
                        IDEState.IDEDev[count].Signature = *((unsigned short *)(&IDEState.Buff[ATA_IDENT_DEVICETYPE]));
                        IDEState.IDEDev[count].Capabilities = *((unsigned short *)(&IDEState.Buff[ATA_IDENT_CAPABILITIES]));
                        IDEState.IDEDev[count].CommandSets = *((uint32_t *)(&IDEState.Buff[ATA_IDENT_COMMANDSETS]));

                        if (IDEState.IDEDev[count].CommandSets & (1 << 26))
                                IDEState.IDEDev[count].Size = *((uint32_t *)(&IDEState.Buff[ATA_IDENT_MAX_LBA_EXT]));
                        else
                                IDEState.IDEDev[count].Size = *((uint32_t *)(&IDEState.Buff[ATA_IDENT_MAX_LBA]));

                        for (k = 0; k < 40; k += 2)
                        {
                                IDEState.IDEDev[count].Model[k] = IDEState.Buff[ATA_IDENT_MODEL + k + 1];
                                IDEState.IDEDev[count].Model[k + 1] = IDEState.Buff[ATA_IDENT_MODEL + k];
                        }
                        IDEState.IDEDev[count].Model[40] = 0;

                        count++;
                }
        // for (i = 0; i < 4; i++)
        //         if (IDEState.IDEDev[i].Reserved == 1)
        //         {
        //                 printf(" Found %s Drive %d - %s\n",
        //                        (const char *[]){"ATA", "ATAPI"}[IDEState.IDEDev[i].Type],
        //                        IDEState.IDEDev[i].Size,
        //                        IDEState.IDEDev[i].Model);
        //         }
}

unsigned char IDEAccess(unsigned char direction, unsigned char drive, unsigned int lba,
                        unsigned char numsects, unsigned short selector, unsigned int edi)
{
        unsigned char lba_mode /* 0: CHS, 1:LBA28, 2: LBA48 */, dma /* 0: No DMA, 1: DMA */, cmd;
        unsigned char lba_io[6];
        unsigned int channel = IDEState.IDEDev[drive].Channel; // Read the Channel.
        unsigned int slavebit = IDEState.IDEDev[drive].Drive;  // Read the Drive [Master/Slave]
        unsigned int bus = IDEState.Channels[channel].base;    // Bus Base, like 0x1F0 which is also data port.
        unsigned int words = 256;
        unsigned short cyl, i;
        unsigned char head, sect, err;
        IDEWrite(channel, ATA_REG_CONTROL, IDEState.Channels[channel].nIEN = (IDEState.Invoked = 0x0) + 0x02);
        if (lba >= 0x10000000)
        {
                lba_mode = 2;
                lba_io[0] = (lba & 0x000000FF) >> 0;
                lba_io[1] = (lba & 0x0000FF00) >> 8;
                lba_io[2] = (lba & 0x00FF0000) >> 16;
                lba_io[3] = (lba & 0xFF000000) >> 24;
                lba_io[4] = 0;
                lba_io[5] = 0;
                head = 0;
        }
        else if (IDEState.IDEDev[drive].Capabilities & 0x200)
        {
                lba_mode = 1;
                lba_io[0] = (lba & 0x00000FF) >> 0;
                lba_io[1] = (lba & 0x000FF00) >> 8;
                lba_io[2] = (lba & 0x0FF0000) >> 16;
                lba_io[3] = 0; // These Registers are not used here.
                lba_io[4] = 0; // These Registers are not used here.
                lba_io[5] = 0; // These Registers are not used here.
                head = (lba & 0xF000000) >> 24;
        }
        else
        {
                lba_mode = 0;
                sect = (lba % 63) + 1;
                cyl = (lba + 1 - sect) / (16 * 63);
                lba_io[0] = sect;
                lba_io[1] = (cyl >> 0) & 0xFF;
                lba_io[2] = (cyl >> 8) & 0xFF;
                lba_io[3] = 0;
                lba_io[4] = 0;
                lba_io[5] = 0;
                head = (lba + 1 - sect) % (16 * 63) / (63);
        }
        dma = 0;
        while (IDERead(channel, ATA_REG_STATUS) & ATA_SR_BSY)
        {
        }
        if (lba_mode == 0)
                IDEWrite(channel, ATA_REG_HDDEVSEL, 0xA0 | (slavebit << 4) | head);
        else
                IDEWrite(channel, ATA_REG_HDDEVSEL, 0xE0 | (slavebit << 4) | head);
        if (lba_mode == 2)
        {
                IDEWrite(channel, ATA_REG_SECCOUNT1, 0);
                IDEWrite(channel, ATA_REG_LBA3, lba_io[3]);
                IDEWrite(channel, ATA_REG_LBA4, lba_io[4]);
                IDEWrite(channel, ATA_REG_LBA5, lba_io[5]);
        }
        IDEWrite(channel, ATA_REG_SECCOUNT0, numsects);
        IDEWrite(channel, ATA_REG_LBA0, lba_io[0]);
        IDEWrite(channel, ATA_REG_LBA1, lba_io[1]);
        IDEWrite(channel, ATA_REG_LBA2, lba_io[2]);
        if (lba_mode == 0 && dma == 0 && direction == 0)
                cmd = ATA_CMD_READ_PIO;
        if (lba_mode == 1 && dma == 0 && direction == 0)
                cmd = ATA_CMD_READ_PIO;
        if (lba_mode == 2 && dma == 0 && direction == 0)
                cmd = ATA_CMD_READ_PIO_EXT;
        if (lba_mode == 0 && dma == 1 && direction == 0)
                cmd = ATA_CMD_READ_DMA;
        if (lba_mode == 1 && dma == 1 && direction == 0)
                cmd = ATA_CMD_READ_DMA;
        if (lba_mode == 2 && dma == 1 && direction == 0)
                cmd = ATA_CMD_READ_DMA_EXT;
        if (lba_mode == 0 && dma == 0 && direction == 1)
                cmd = ATA_CMD_WRITE_PIO;
        if (lba_mode == 1 && dma == 0 && direction == 1)
                cmd = ATA_CMD_WRITE_PIO;
        if (lba_mode == 2 && dma == 0 && direction == 1)
                cmd = ATA_CMD_WRITE_PIO_EXT;
        if (lba_mode == 0 && dma == 1 && direction == 1)
                cmd = ATA_CMD_WRITE_DMA;
        if (lba_mode == 1 && dma == 1 && direction == 1)
                cmd = ATA_CMD_WRITE_DMA;
        if (lba_mode == 2 && dma == 1 && direction == 1)
                cmd = ATA_CMD_WRITE_DMA_EXT;
        IDEWrite(channel, ATA_REG_COMMAND, cmd);
        if (dma)
                if (direction == 0)
                        ;
                else
                        ;
        else if (direction == 0)
                for (i = 0; i < numsects; i++)
                {
                        if ((err = IDEPolling(channel, 1)))
                                return err;
                        asm("pushw %es");
                        asm("mov %%ax, %%es" : : "a"(selector));
                        asm("rep insw" : : "c"(words), "d"(bus), "D"(edi));
                        asm("popw %es");
                        edi += (words * 2);
                }
        else
        {
                for (i = 0; i < numsects; i++)
                {
                        IDEPolling(channel, 0);
                        asm("pushw %ds");
                        asm("mov %%ax, %%ds" ::"a"(selector));
                        asm("rep outsw" ::"c"(words), "d"(bus), "S"(edi));
                        asm("popw %ds");
                        edi += (words * 2);
                }
                IDEWrite(channel, ATA_REG_COMMAND, (char[]){ATA_CMD_CACHE_FLUSH, ATA_CMD_CACHE_FLUSH, ATA_CMD_CACHE_FLUSH_EXT}[lba_mode]);
                IDEPolling(channel, 0);
        }

        return 0;
}

unsigned char IDEAtaPIRead(unsigned char drive, unsigned int lba, unsigned char numsects,
                           unsigned short selector, unsigned int edi)
{
        sti();
        unsigned int channel = IDEState.IDEDev[drive].Channel;
        unsigned int slavebit = IDEState.IDEDev[drive].Drive;
        unsigned int bus = IDEState.Channels[channel].base;
        unsigned int words = 1024; // Sector Size. ATAPI drives have a sector size of 2048 bytes.
        unsigned char err;
        int i;
        IDEWrite(channel, ATA_REG_CONTROL, IDEState.Channels[channel].nIEN = IDEState.Invoked = 0x0);
        atapi_packet[0] = ATAPI_CMD_READ;
        atapi_packet[1] = 0x0;
        atapi_packet[2] = (lba >> 24) & 0xFF;
        atapi_packet[3] = (lba >> 16) & 0xFF;
        atapi_packet[4] = (lba >> 8) & 0xFF;
        atapi_packet[5] = (lba >> 0) & 0xFF;
        atapi_packet[6] = 0x0;
        atapi_packet[7] = 0x0;
        atapi_packet[8] = 0x0;
        atapi_packet[9] = numsects;
        atapi_packet[10] = 0x0;
        atapi_packet[11] = 0x0;

        IDEWrite(channel, ATA_REG_HDDEVSEL, slavebit << 4);

        for (int i = 0; i < 4; i++)
                IDERead(channel, ATA_REG_ALTSTATUS);
        IDEWrite(channel, ATA_REG_FEATURES, 0);
        IDEWrite(channel, ATA_REG_LBA1, (words * 2) & 0xFF); // Lower Byte of Sector Size.
        IDEWrite(channel, ATA_REG_LBA2, (words * 2) >> 8);

        IDEWrite(channel, ATA_REG_COMMAND, ATA_CMD_PACKET); // Send the Command.
        if ((err = IDEPolling(channel, 1)))
                return err;
        asm("rep   outsw" : : "c"(6), "d"(bus), "S"(atapi_packet)); // Send Packet Data
        for (i = 0; i < numsects; i++)
        {
                IDEWaitIRQ(); // Wait for an IRQ.
                if ((err = IDEPolling(channel, 1)))
                        return err; // Polling and return if error.
                asm("pushw %es");
                asm("mov %%ax, %%es" ::"a"(selector));
                asm("rep insw" ::"c"(words), "d"(bus), "D"(edi)); // Receive Data.
                asm("popw %es");
                edi += (words * 2);
        }
        IDEWaitIRQ();

        while (IDERead(channel, ATA_REG_STATUS) & (ATA_SR_BSY | ATA_SR_DRQ))
                ;

        return 0;
}

void IDEReadSectors(unsigned char drive, unsigned char numsects, unsigned int lba,
                    unsigned short es, unsigned int edi)
{
        int i;
        if (drive > 3 || IDEState.IDEDev[drive].Reserved == 0)
                package[0] = 0x1;
        else if (((lba + numsects) > IDEState.IDEDev[drive].Size) && (IDEState.IDEDev[drive].Type == IDE_ATA))
                package[0] = 0x2;
        else
        {
                unsigned char err = 0;
                if (IDEState.IDEDev[drive].Type == IDE_ATA)
                        err = IDEAccess(ATA_READ, drive, lba, numsects, es, edi);
                else if (IDEState.IDEDev[drive].Type == IDE_ATAPI)
                        for (i = 0; i < numsects; i++)
                                err = IDEAtaPIRead(drive, lba + i, 1, es, edi + (i * 2048));
                package[0] = IDEPrintErr(drive, err);
        }
}

void IDEWriteSectors(unsigned char drive, unsigned char numsects, unsigned int lba,
                       unsigned short es, unsigned int edi)
{
        if (drive > 3 || IDEState.IDEDev[drive].Reserved == 0)
                package[0] = 0x1;
        else if (((lba + numsects) > IDEState.IDEDev[drive].Size) && (IDEState.IDEDev[drive].Type == IDE_ATA))
                package[0] = 0x2;
        else
        {
                unsigned char err = 0;
                if (IDEState.IDEDev[drive].Type == IDE_ATA)
                        err = IDEAccess(ATA_WRITE, drive, lba, numsects, es, edi);
                else if (IDEState.IDEDev[drive].Type == IDE_ATAPI)
                        err = 4;
                package[0] = IDEPrintErr(drive, err);
        }
}

void IDEFind(size_t Index)
{
        cli();
        IDEState.Dev = pciGetOriginalDevice(Index);
        if (!IDEState.Dev || IDEState.Dev->class_id != 0x01 || IDEState.Dev->subclass_id != 0x01)
        {
                SerialPrint("ERROR: Could not find IDE Device\r\n");
                return;
        }

        SerialPrint(" [INFO] IDE Device found\r\n");
        IDEInitialise();
}
