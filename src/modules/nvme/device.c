#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/vmm.h>
#include <kernel/pci/pci.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <nvme/device.h>
#include <nvme/registers.h>
#define KERNEL_LOG_NAME "nvme"



static void _nvme_init_device(pci_device_t* device){
	if (device->class!=0x01||device->subclass!=0x08||device->progif!=0x02){
		return;
	}
	pci_device_enable_memory_access(device);
	pci_device_enable_bus_mastering(device);
	pci_bar_t pci_bar;
	if (!pci_device_get_bar(device,0,&pci_bar)){
		return;
	}
	LOG("Attached NVMe driver to PCI device %x:%x:%x",device->address.bus,device->address.slot,device->address.func);
	nvme_registers_t* registers=(void*)vmm_identity_map(pci_bar.address,sizeof(nvme_registers_t));
	if (!(registers->cap&0x0000002000000000ull)){
		WARN("NVMe instruction set not supported");
		return;
	}
	INFO("NVMe version %x.%x.%x",registers->vs>>16,(registers->vs>>8)&0xff,registers->vs&0xff);
	INFO("Min page size: %lu, Max page size: %lu",1<<(12+((registers->cap>>48)&15)),1<<(12+((registers->cap>>52)&15)));
	registers->cc&=~CC_EN;
	SPINLOOP(registers->csts&CSTS_RDY);
	u32 queue_entries=(registers->cap&0xffff)+1;
	u8 doorbell_stride=4<<((registers->cap>>32)&0xf);
	INFO("Queue entry count: %u, Doorbell stride: %v",queue_entries,doorbell_stride);
}



void nvme_locate_devices(void){
	HANDLE_FOREACH(HANDLE_TYPE_PCI_DEVICE){
		pci_device_t* device=handle->object;
		_nvme_init_device(device);
	}
}
