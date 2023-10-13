#include <kernel/driver/xhci.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/pci/pci.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "xhci"




#define XHCI_RING_SIZE 16

// Command Ring Control Register
#define CRCR_RCS 1

// USB Command
#define USBCMD_RS 1
#define USBCMD_HCRST 2

// USB Status
#define USBSTS_HCH 1
#define USBSTS_CNR 2048



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
	u8 _padding[8];
	u32 dnctrl;
	u64 crcr;
	u8 _padding2[16];
	u64 dcbaap;
	u32 config;
} xhci_operational_registers_t;



typedef volatile struct _XHCI_PORT_REGISTERS{
	u32 portsc;
	u32 portpmsc;
	u32 portli;
	u8 _padding[4];
} xhci_port_registers_t;



typedef volatile struct _XHCI_DEVICE_CONTEXT_BASE{
	u64 ptr;
} xhci_device_context_base_t;



typedef volatile struct _XHCI_TRANSFER_BLOCK{
	u64 ptr;
	u32 status;
	u32 control;
} xhci_transfer_block_t;



typedef struct _XHCI_DEVICE{
	xhci_registers_t* registers;
	xhci_operational_registers_t* operational_registers;
	xhci_port_registers_t* port_registers;
	xhci_device_context_base_t* device_context_base_array;
	xhci_transfer_block_t* command_ring;
	xhci_transfer_block_t* event_ring;
	u8 ports;
	u8 slots;
	u8 context_size;
} xhci_device_t;



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
	if (operational_registers->pagesize!=1){
		WARN("Page count not supported");
		return;
	}
	xhci_device_t* xhci_device=kmm_alloc(sizeof(xhci_device_t));
	xhci_device->registers=registers;
	xhci_device->operational_registers=operational_registers;
	xhci_device->ports=registers->hcsparams1>>24;
	xhci_device->slots=registers->hcsparams1;
	xhci_device->context_size=((registers->hccparams1&0x04)?64:32);
	xhci_device->port_registers=(void*)vmm_identity_map(pci_bar.address+0x400,xhci_device->ports*sizeof(xhci_port_registers_t));
	INFO("Ports: %u, Slots: %u, Context size: %u",xhci_device->ports,xhci_device->slots,xhci_device->context_size);
	xhci_device->device_context_base_array=kmm_alloc_aligned((xhci_device->slots+1)+sizeof(xhci_device_context_base_t),64);
	xhci_device->command_ring=kmm_alloc_aligned(XHCI_RING_SIZE*sizeof(xhci_transfer_block_t),64);
	xhci_device->event_ring=kmm_alloc_aligned(XHCI_RING_SIZE*sizeof(xhci_transfer_block_t),64);
	if (xhci_device->operational_registers->usbcmd&USBCMD_RS){
		xhci_device->operational_registers->usbcmd&=~USBCMD_RS;
		SPINLOOP(!(xhci_device->operational_registers->usbsts&USBSTS_HCH));
	}
	xhci_device->operational_registers->usbcmd=USBCMD_HCRST;
	SPINLOOP(xhci_device->operational_registers->usbcmd&USBCMD_HCRST);
	SPINLOOP(xhci_device->operational_registers->usbsts&USBSTS_CNR);
	// Initialization described in section 4.2
	xhci_device->operational_registers->config=xhci_device->slots;
	xhci_device->operational_registers->dcbaap=(u64)(xhci_device->device_context_base_array);
	xhci_device->operational_registers->crcr=((u64)(xhci_device->command_ring))|CRCR_RCS;
	xhci_device->operational_registers->usbcmd|=USBCMD_RS;
	panic("AAA");
}
