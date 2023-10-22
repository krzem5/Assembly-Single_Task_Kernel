#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/module/module.h>
#include <kernel/pci/pci.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "nvme"



// Controller Configuration flags
#define CC_EN 0x01

// Controller Status flags
#define CSTS_RDY 0x01



typedef volatile struct _NVME_REGISTERS{
	u64 cap;
	u32 vs;
	u32 intms;
	u32 intmc;
	u32 cc;
	u32 rsvd1;
	u32 csts;
	u32 rsvd2;
	u32 aqa;
	u64 asq;
	u64 acq;
} nvme_registers_t;



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



static _Bool _init(module_t* module){
	HANDLE_FOREACH(HANDLE_TYPE_PCI_DEVICE){
		pci_device_t* device=handle->object;
		_nvme_init_device(device);
	}
	return 1;
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	"nvme",
	_init,
	_deinit
);
