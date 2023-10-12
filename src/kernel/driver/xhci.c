#include <kernel/driver/xhci.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/pci/pci.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "xhci"



typedef volatile struct _XHCI_REGISTERS{
	u8 caplength;
	u8 _padding;
	u16 hciversion;
	u32 hcsparams1;
	u32 hcsparams2;
	u32 hcsparams3;
	u32 hccparams1;
	u32 dboff;
	u32 rtsoff;
	u32 hccparams2;
} xhci_registers_t;



typedef volatile struct _XHCI_OPERATIONAL_REGISTERS{
	u32 usbcmd;
	u32 usbsts;
	u32 pagesize;
	u8 _padding2[8];
	u32 dnctrl;
	u32 crcr;
	u8 _padding3[20];
	u64 dcbaap;
	u64 config;
} xhci_operational_registers_t;



void driver_xhci_init_device(pci_device_t* device){
	if (device->class!=0x0c||device->subclass!=0x03||device->progif!=0x30){
		return;
	}
	pci_device_enable_memory_access(device);
	pci_device_enable_bus_mastering(device);
	pci_bar_t pci_bar;
	if (!pci_device_get_bar(device,0,&pci_bar)){
		return;
	}
	LOG("Attached XHCI driver to PCI device %x:%x:%x",device->bus,device->slot,device->func);
	xhci_registers_t* registers=(void*)vmm_identity_map(pci_bar.address,sizeof(xhci_registers_t));
	xhci_operational_registers_t* operational_registers=(void*)vmm_identity_map(pci_bar.address+registers->caplength,sizeof(xhci_operational_registers_t));
	INFO("Ports: %u, Slots: %u, Context size: %u",registers->hcsparams1>>24,registers->hcsparams1&0xff,((registers->hccparams1&0x04)?64:32));
	if (operational_registers->pagesize!=1){
		WARN("Page count not supported");
		return;
	}
}
