#ifndef _KERNEL_DRIVER_NVME_H_
#define _KERNEL_DRIVER_NVME_H_
#include <kernel/pci/pci.h>



void driver_nvme_init(void);



void driver_nvme_init_device(pci_device_t* device);



#endif
