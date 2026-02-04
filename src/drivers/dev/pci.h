#ifndef PCI_H
#define PCI_H

#include <stdint.h>
#include <stdarg.h>

#include <tty/output.h>
#include <utils.h>
#include <io.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC
#define MAX_DEVICES 4096

typedef struct
{
        uint8_t bus;
        uint8_t slot;
        uint8_t function;
        uint16_t vendor_id;
        uint16_t device_id;
        uint8_t class_id;
        uint8_t subclass_id;
        uint8_t prog_if;
        uint8_t revision;
        uint8_t header_type;
        uint8_t irq_line;
        uint32_t bar[6];
} pci_device_t;

uint32_t pciGetDeviceCount(void);
size_t pciGetDevices(pci_device_t *destination, size_t start, size_t end);
const char *pciClassToString(uint8_t class_id, uint8_t subclass_id);
void pciEnumerateDevices(void (*on_device_found)(pci_device_t *));
void pciDisplayDeviceInfo(pci_device_t *dev);
void pciRegister(pci_device_t *dev);
pci_device_t *pciFindOfType(uint8_t class_id, uint8_t subclass_id);
pci_device_t *pciGetOriginalDevice(size_t Index);

#endif // PCI_H
