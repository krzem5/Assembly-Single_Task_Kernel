#include <kernel/driver/nvme.h>
#include <kernel/log/log.h>
#include <kernel/pci/pci.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "nvme"



void KERNEL_CORE_CODE driver_nvme_init(void){
	return;
}



void KERNEL_CORE_CODE driver_nvme_init_device(pci_device_t* device){
	if (device->class!=0x01||device->subclass!=0x08||device->progif!=0x02){
		return;
	}
	pci_device_enable_memory_access(device);
	pci_device_enable_bus_mastering(device);
	pci_bar_t pci_bar;
	if (!pci_device_get_bar(device,0,&pci_bar)){
		return;
	}
	LOG_CORE("Attached NVMe driver to PCI device %x:%x:%x",device->bus,device->slot,device->func);
}
