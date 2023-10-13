#include <kernel/driver/xhci.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/pci/pci.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "xhci"



PMM_DECLARE_COUNTER(DRIVER_XHCI);
PMM_DECLARE_COUNTER(OMM_USB_DEVICE);



#define XHCI_RING_SIZE 16

// Command Ring Control Register
#define CRCR_RCS 1

// USB Command
#define USBCMD_RS 1
#define USBCMD_HCRST 2

// USB Status
#define USBSTS_HCH 1
#define USBSTS_CNR 2048

// Port Status and Control Register
#define PORTSC_CCS 1
#define PORTSC_PLS_MASK 480
#define PORTSC_PLS_U0 0
#define PORTSC_PED 2
#define PORTSC_SPEED_MASK 15360
#define PORTSC_SPEED_FULL 1024
#define PORTSC_SPEED_LOW 2048
#define PORTSC_SPEED_HIGH 3072
#define PORTSC_SPEED_SUPER 4096



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



typedef volatile struct _XHCI_DOORBELL{
	u32 value;
} xhci_doorbell_t;



typedef volatile struct _XHCI_INTERRUPT_REGISTERS{
	u32 iman;
	u32 imod;
	u32 erstsz;
	u8 _padding[4];
	u64 erstba;
	u64 erdp;
} xhci_interrupt_registers_t;



typedef volatile struct _XHCI_DEVICE_CONTEXT_BASE{
	u64 ptr;
} xhci_device_context_base_t;



typedef volatile struct _XHCI_TRANSFER_BLOCK{
	u64 ptr;
	u32 status;
	u32 control;
} xhci_transfer_block_t;



typedef volatile struct _XHCI_EVENT_RING_SEGMENT{
    u64 ptr;
    u32 size;
    u8 _padding[4];
} xhci_event_ring_segment_t;



typedef struct _XHCI_DEVICE{
	xhci_registers_t* registers;
	xhci_operational_registers_t* operational_registers;
	xhci_port_registers_t* port_registers;
	xhci_doorbell_t* doorbell_registers;
	xhci_interrupt_registers_t* interrupt_registers;
	xhci_device_context_base_t* device_context_base_array;
	xhci_transfer_block_t* command_ring;
	xhci_transfer_block_t* event_ring;
	xhci_event_ring_segment_t* event_ring_segment;
	u8 ports;
	u16 interrupts;
	u8 slots;
	u8 context_size;
} xhci_device_t;



#define USB_DEVICE_SPEED_INVALID 0
#define USB_DEVICE_SPEED_FULL 1
#define USB_DEVICE_SPEED_LOW 2
#define USB_DEVICE_SPEED_HIGH 3
#define USB_DEVICE_SPEED_SUPER 4

#define USB_DEVICE_TYPE_HUB 0



typedef struct _USB_CONTROLLER{
	void* device;
	_Bool (*detect)(void*,u16);
	u8 (*reset)(void*,u16);
	void (*disconnect)(void*,u16);
} usb_controller_t;



typedef struct _USB_DEVICE{
	const usb_controller_t* controller;
	struct _USB_DEVICE* parent;
	struct _USB_DEVICE* prev;
	struct _USB_DEVICE* next;
	u8 type;
	u8 speed;
	u16 port;
	union{
		struct{
			u16 port_count;
		} hub;
	};
} usb_device_t;



static omm_allocator_t _usb_device_allocator=OMM_ALLOCATOR_INIT_STRUCT("usb_device",sizeof(usb_device_t),8,4,PMM_COUNTER_OMM_USB_DEVICE);



static KERNEL_INLINE u32 _align_size(u32 size){
	return (size+63)&0xffffffc0;
}



static u32 _get_total_memory_size(const xhci_device_t* device){
	u32 out=0;
	out+=_align_size((device->slots+1)*sizeof(xhci_device_context_base_t));
	out+=_align_size(XHCI_RING_SIZE*sizeof(xhci_transfer_block_t));
	out+=_align_size(XHCI_RING_SIZE*sizeof(xhci_transfer_block_t));
	out+=_align_size(sizeof(xhci_event_ring_segment_t));
	return out;
}



static void _enumerate_hub(usb_device_t* hub){
	for (u16 i=0;i<hub->hub.port_count;i++){
		if (!hub->controller->detect(hub->controller->device,i)){
			continue;
		}
		u8 speed=hub->controller->reset(hub->controller->device,i);
		if (speed==USB_DEVICE_SPEED_INVALID){
			continue;
		}
		LOG("Port: %u, Speed: %u",i,speed);
	}
}



static _Bool _xhci_detect_port(void* ctx,u16 port){
	const xhci_device_t* xhci_device=ctx;
	return !!((xhci_device->port_registers+port)->portsc&PORTSC_CCS);
}



static u8 _xhci_reset_port(void* ctx,u16 port){
	const xhci_device_t* xhci_device=ctx;
	xhci_port_registers_t* port_registers=xhci_device->port_registers+port;
	if (!(port_registers->portsc&PORTSC_CCS)||(port_registers->portsc&PORTSC_PLS_MASK)!=PORTSC_PLS_U0){
		return USB_DEVICE_SPEED_INVALID;
	}
	SPINLOOP((port_registers->portsc&(PORTSC_CCS|PORTSC_PED))==PORTSC_CCS);
	if (!(port_registers->portsc&PORTSC_PED)){
		return USB_DEVICE_SPEED_INVALID;
	}
	switch (port_registers->portsc&PORTSC_SPEED_MASK){
		case PORTSC_SPEED_FULL:
			return USB_DEVICE_SPEED_FULL;
		case PORTSC_SPEED_LOW:
			return USB_DEVICE_SPEED_LOW;
		case PORTSC_SPEED_HIGH:
			return USB_DEVICE_SPEED_HIGH;
		case PORTSC_SPEED_SUPER:
			return USB_DEVICE_SPEED_SUPER;
	}
	return USB_DEVICE_SPEED_INVALID;
}



static void _xhci_disconnect_port(void* ctx,u16 port){
	panic("_xhci_disconnect_port");
}



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
		WARN("Page size not supported");
		return;
	}
	xhci_device_t* xhci_device=kmm_alloc(sizeof(xhci_device_t));
	xhci_device->registers=registers;
	xhci_device->operational_registers=operational_registers;
	xhci_device->ports=registers->hcsparams1>>24;
	xhci_device->interrupts=(registers->hcsparams1>>8)&0x7ff;
	xhci_device->slots=registers->hcsparams1;
	xhci_device->context_size=((registers->hccparams1&0x04)?64:32);
	xhci_device->port_registers=(void*)vmm_identity_map(pci_bar.address+0x400,xhci_device->ports*sizeof(xhci_port_registers_t));
	xhci_device->doorbell_registers=(void*)vmm_identity_map(pci_bar.address+xhci_device->registers->dboff,xhci_device->ports*sizeof(xhci_doorbell_t));
	xhci_device->interrupt_registers=(void*)vmm_identity_map(pci_bar.address+xhci_device->registers->rtsoff+0x20,xhci_device->interrupts*sizeof(xhci_interrupt_registers_t));
	INFO("Ports: %u, Interrupts: %u, Slots: %u, Context size: %u",xhci_device->ports,xhci_device->interrupts,xhci_device->slots,xhci_device->context_size);
	void* data=(void*)(pmm_alloc_zero(pmm_align_up_address(_get_total_memory_size(xhci_device))>>PAGE_SIZE_SHIFT,PMM_COUNTER_DRIVER_XHCI,PMM_MEMORY_HINT_LOW_MEMORY)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	xhci_device->device_context_base_array=data;
	data+=_align_size((xhci_device->slots+1)*sizeof(xhci_device_context_base_t));
	xhci_device->command_ring=data;
	data+=_align_size(XHCI_RING_SIZE*sizeof(xhci_transfer_block_t));
	xhci_device->event_ring=data;
	data+=_align_size(XHCI_RING_SIZE*sizeof(xhci_transfer_block_t));
	xhci_device->event_ring_segment=data;
	if (xhci_device->operational_registers->usbcmd&USBCMD_RS){
		xhci_device->operational_registers->usbcmd&=~USBCMD_RS;
		SPINLOOP(!(xhci_device->operational_registers->usbsts&USBSTS_HCH));
	}
	xhci_device->operational_registers->usbcmd=USBCMD_HCRST;
	SPINLOOP(xhci_device->operational_registers->usbcmd&USBCMD_HCRST);
	SPINLOOP(xhci_device->operational_registers->usbsts&USBSTS_CNR);
	xhci_device->event_ring_segment->ptr=((u64)(xhci_device->event_ring))-VMM_HIGHER_HALF_ADDRESS_OFFSET;
	xhci_device->event_ring_segment->size=XHCI_RING_SIZE;
	xhci_device->operational_registers->config=xhci_device->slots;
	xhci_device->operational_registers->dcbaap=((u64)(xhci_device->device_context_base_array))-VMM_HIGHER_HALF_ADDRESS_OFFSET;
	xhci_device->operational_registers->crcr=((u64)(xhci_device->command_ring))-VMM_HIGHER_HALF_ADDRESS_OFFSET+CRCR_RCS;
	xhci_device->interrupt_registers->erstsz=1;
	xhci_device->interrupt_registers->erstba=((u64)(xhci_device->event_ring_segment))-VMM_HIGHER_HALF_ADDRESS_OFFSET;
	xhci_device->interrupt_registers->erdp=((u64)(xhci_device->event_ring))-VMM_HIGHER_HALF_ADDRESS_OFFSET;
	u32 spb=xhci_device->registers->hcsparams2;
	spb=(spb>>27)|((spb>>16)&0x3e0);
	if (spb){
		WARN("Setup XHCI scratchpad of size %u",spb);
	}
	xhci_device->operational_registers->usbcmd|=USBCMD_RS;
	COUNTER_SPINLOOP(0xfff);
	usb_controller_t* controller=kmm_alloc(sizeof(usb_controller_t));
	controller->device=xhci_device;
	controller->detect=_xhci_detect_port;
	controller->reset=_xhci_reset_port;
	controller->disconnect=_xhci_disconnect_port;
	usb_device_t* root_hub=omm_alloc(&_usb_device_allocator);
	root_hub->controller=controller;
	root_hub->parent=NULL;
	root_hub->prev=NULL;
	root_hub->next=NULL;
	root_hub->type=USB_DEVICE_TYPE_HUB;
	root_hub->port=0;
	root_hub->hub.port_count=xhci_device->ports;
	_enumerate_hub(root_hub);
	// panic("Test");
}
