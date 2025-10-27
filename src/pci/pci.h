#ifndef PCI_H
#define PCI_H

#include <stdint.h>
#include <stdarg.h>

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

extern const char *pciClassToString(uint8_t class_id, uint8_t subclass_id);
extern void pciEnumerateDevices(void (*on_device_found)(pci_device_t *));
extern void print_pci_device(pci_device_t *dev);
void register_device(pci_device_t *dev);
pci_device_t *find_device_of_type(uint8_t class_id, uint8_t subclass_id);

extern pci_device_t devices[MAX_DEVICES];
extern int device_count;

#endif // PCI_H
