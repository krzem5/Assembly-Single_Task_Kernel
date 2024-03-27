#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/pci/pci.h>
#include <kernel/types.h>
#include <kernel/usb/controller.h>
#include <kernel/usb/device.h>
#include <kernel/usb/structures.h>
#include <kernel/util/spinloop.h>
#include <kernel/util/util.h>
#include <xhci/device.h>
#include <xhci/registers.h>
#define KERNEL_LOG_NAME "xhci"



static pmm_counter_descriptor_t* _xhci_driver_pmm_counter=NULL;
static pmm_counter_descriptor_t* _xhci_input_context_pmm_counter=NULL;
static omm_allocator_t* _xhci_device_allocator=NULL;
static omm_allocator_t* _xhci_ring_allocator=NULL;
static omm_allocator_t* _xhci_pipe_allocator=NULL;



static KERNEL_INLINE u32 _align_size(u32 size){
	return (size+63)&0xffffffc0;
}



static u32 _get_total_memory_size(const xhci_device_t* device){
	u32 out=0;
	out+=_align_size((device->slots+1)*sizeof(xhci_device_context_base_t));
	out+=_align_size(sizeof(xhci_event_ring_segment_t));
	return out;
}



static xhci_ring_t* _alloc_ring(_Bool cs){
	xhci_ring_t* out=omm_alloc(_xhci_ring_allocator);
	mem_fill(out,0,sizeof(xhci_ring_t));
	spinlock_init(&(out->lock));
	out->cs=cs;
	return out;
}



static u8 _speed_to_context_speed(u8 speed){
	switch (speed){
		case USB_DEVICE_SPEED_FULL:
			return 1;
		case USB_DEVICE_SPEED_LOW:
			return 2;
		case USB_DEVICE_SPEED_HIGH:
			return 3;
		case USB_DEVICE_SPEED_SUPER:
			return 4;
	}
	panic("_speed_to_context_speed: invalid speed");
}



static xhci_input_context_t* _alloc_input_context_raw(xhci_device_t* xhci_device){
	return (void*)(pmm_alloc(pmm_align_up_address((sizeof(xhci_input_context_t)<<xhci_device->is_context_64_bytes)*33)>>PAGE_SIZE_SHIFT,_xhci_input_context_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
}



static xhci_input_context_t* _alloc_input_context(xhci_device_t* xhci_device,usb_device_t* device,u8 max_endpoint){
	xhci_input_context_t* out=_alloc_input_context_raw(xhci_device);
	xhci_input_context_t* slot=out+(1<<xhci_device->is_context_64_bytes);
	slot->slot.ctx[0]=(max_endpoint<<27)|(_speed_to_context_speed(device->speed)<<20);
	slot->slot.ctx[1]=(device->port+1)<<16;
	if (!device->parent->parent){
		return out;
	}
	if (device->speed==USB_DEVICE_SPEED_FULL||device->speed==USB_DEVICE_SPEED_LOW){
		panic("_alloc_input_context: slow USB device connected to an XHCI device");
	}
	u32 route=0;
	for (;device->parent;device=device->parent){
		route=(route<<4)|((device->port+1)&0xf);
	}
	slot->slot.ctx[0]|=route;
	return out;
}



static void _dealloc_input_context(xhci_device_t* xhci_device,xhci_input_context_t* input_context){
	pmm_dealloc(((u64)input_context)-VMM_HIGHER_HALF_ADDRESS_OFFSET,pmm_align_up_address((sizeof(xhci_input_context_t)<<xhci_device->is_context_64_bytes)*33)>>PAGE_SIZE_SHIFT,_xhci_input_context_pmm_counter);
}



static void _enqueue_event_raw(xhci_ring_t* ring,const void* data,u32 size,u32 flags){
	xhci_transfer_block_t* transfer_block=ring->ring+ring->nidx;
	if (flags&TRB_IDT){
		if (size>8){
			panic("_enqueue_event_raw: inline size too large");
		}
		mem_copy((void*)(transfer_block->inline_data),data,size);
	}
	else if (!data){
		transfer_block->address=0;
	}
	else{
		transfer_block->address=vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)data);
		for (u64 i=PAGE_SIZE;i<size;i+=PAGE_SIZE){
			if (vmm_virtual_to_physical(&vmm_kernel_pagemap,((u64)data)+i)!=transfer_block->address+i){
				panic("_enqueue_event_raw: data crosses page boundary");
			}
		}
	}
	transfer_block->status=size;
	transfer_block->flags=flags|ring->cs;
}



static void _enqueue_event(xhci_ring_t* ring,const void* data,u32 size,u32 flags){
	if (ring->nidx>=XHCI_RING_SIZE-1){
		_enqueue_event_raw(ring,(void*)(ring->ring),0,TRB_TYPE_TR_LINK|TRB_LK_TC);
		ring->nidx=0;
		ring->cs^=1;
	}
	_enqueue_event_raw(ring,data,size,flags);
	ring->nidx++;
}



static u8 _wait_for_all_events(xhci_device_t* xhci_device,xhci_ring_t* ring){
	while (ring->eidx!=ring->nidx){
		SPINLOOP(((xhci_device->event_ring->ring+xhci_device->event_ring->nidx)->flags&1)!=xhci_device->event_ring->cs);
		xhci_transfer_block_t* transfer_block=xhci_device->event_ring->ring+xhci_device->event_ring->nidx;
		u32 type=transfer_block->flags&TRB_TYPE_MASK;
		if (type==TRB_TYPE_ER_TRANSFER||type==TRB_TYPE_ER_COMMAND_COMPLETE){
			xhci_ring_t* ring=(void*)((transfer_block->address&(-XHCI_RING_SIZE*sizeof(xhci_transfer_block_t)))+VMM_HIGHER_HALF_ADDRESS_OFFSET);
			ring->event=*transfer_block;
			ring->eidx=(transfer_block->address&(XHCI_RING_SIZE*sizeof(xhci_transfer_block_t)-1))/sizeof(xhci_transfer_block_t)+1;
		}
		else if (type==TRB_TYPE_ER_PORT_STATUS_CHANGE){
			panic("_wait_for_all_events: port status change");
		}
		else{
			WARN("XHCI: Unknown transfer block type: %u",type>>10);
		}
		xhci_device->event_ring->nidx=(xhci_device->event_ring->nidx+1)&(XHCI_RING_SIZE-1);
		xhci_device->event_ring->cs^=!xhci_device->event_ring->nidx;
		xhci_device->interrupt_registers->erdp=((u64)(xhci_device->event_ring->ring+xhci_device->event_ring->nidx))-VMM_HIGHER_HALF_ADDRESS_OFFSET;
	}
	return ring->event.status>>24;
}



static void _command_submit(xhci_device_t* xhci_device,xhci_input_context_t* input_context,u32 flags){
	spinlock_acquire_exclusive(&(xhci_device->command_ring->lock));
	_enqueue_event(xhci_device->command_ring,(void*)input_context,0,flags);
	xhci_device->doorbell_registers->value=0;
	_wait_for_all_events(xhci_device,xhci_device->command_ring);
	spinlock_release_exclusive(&(xhci_device->command_ring->lock));
}



static u8 _reset_port(void* ctx,u16 port){
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



static usb_pipe_t* _xhci_pipe_alloc(void* ctx,usb_device_t* device,u8 endpoint_address,u8 attributes,u16 max_packet_size){
	xhci_device_t* xhci_device=ctx;
	u8 endpoint_type=attributes&USB_ENDPOINT_XFER_MASK;
	u8 endpoint_id=(endpoint_address?((endpoint_address&0x0f)<<1)|(!!(endpoint_address&USB_DIR_IN)):1);
	xhci_pipe_t* out=omm_alloc(_xhci_pipe_allocator);
	out->ring=_alloc_ring(0);
	out->ring->cs=1;
	out->endpoint_id=endpoint_id;
	xhci_input_context_t* input_context=_alloc_input_context(xhci_device,device,endpoint_id);
	input_context->input.address=(1<<endpoint_id)|1;
	xhci_input_context_t* endpoint=input_context+((endpoint_id+1)<<xhci_device->is_context_64_bytes);
	endpoint->endpoint.ctx[1]=(max_packet_size<<16)|(endpoint_type<<3);
	if (endpoint_type==USB_ENDPOINT_XFER_CONTROL||(endpoint_address&USB_DIR_IN)){
		endpoint->endpoint.ctx[1]|=0x20;
	}
	endpoint->endpoint.deq=(((u64)(out->ring))-VMM_HIGHER_HALF_ADDRESS_OFFSET)|1;
	endpoint->endpoint.length=max_packet_size;
	if (endpoint_id==1){
		if (device->parent->parent){
			panic("_xhci_pipe_alloc: config parent");
		}
		xhci_input_context_t* device_input_context=_alloc_input_context_raw(xhci_device);
		_command_submit(xhci_device,NULL,TRB_TYPE_CR_ENABLE_SLOT);
		out->slot=xhci_device->command_ring->event.flags>>24;
		(xhci_device->device_context_base_array+out->slot)->address=((u64)device_input_context)-VMM_HIGHER_HALF_ADDRESS_OFFSET;
		_command_submit(xhci_device,input_context,(out->slot<<24)|TRB_TYPE_CR_ADDRESS_DEVICE);
	}
	else{
		out->slot=((xhci_pipe_t*)(device->default_pipe))->slot;
		_command_submit(xhci_device,input_context,(out->slot<<24)|TRB_TYPE_CR_CONFIGURE_ENDPOINT);
	}
	_dealloc_input_context(xhci_device,input_context);
	return out;
}



static void _xhci_pipe_resize(void* ctx,usb_device_t* device,usb_pipe_t* pipe,u16 max_packet_size){
	xhci_device_t* xhci_device=ctx;
	xhci_pipe_t* xhci_pipe=pipe;
	xhci_input_context_t* input_context=_alloc_input_context(xhci_device,device,1);
	input_context->input.address=2;
	(input_context+(2<<xhci_device->is_context_64_bytes))->endpoint.ctx[1]=max_packet_size<<16;
	_command_submit(xhci_device,input_context,(xhci_pipe->slot<<24)|TRB_TYPE_CR_EVALUATE_CONTEXT);
	_dealloc_input_context(xhci_device,input_context);
}



static void _xhci_pipe_transfer_setup(void* ctx,usb_device_t* device,usb_pipe_t* pipe,const usb_raw_control_request_t* request,void* data){
	xhci_device_t* xhci_device=ctx;
	xhci_pipe_t* xhci_pipe=pipe;
	spinlock_acquire_exclusive(&(xhci_device->command_ring->lock));
	_enqueue_event(xhci_pipe->ring,request,sizeof(usb_raw_control_request_t),((request->wLength?((!!(request->bRequestType&USB_DIR_IN))+2)<<16:0))|TRB_IDT|TRB_TYPE_TR_SETUP);
	if (request->wLength){
		_enqueue_event(xhci_pipe->ring,data,request->wLength,((!!(request->bRequestType&USB_DIR_IN))<<16)|TRB_TYPE_TR_DATA);
	}
	_enqueue_event(xhci_pipe->ring,NULL,0,((!(request->bRequestType&USB_DIR_IN))<<16)|TRB_IOC|TRB_TYPE_TR_STATUS);
	(xhci_device->doorbell_registers+xhci_pipe->slot)->value=xhci_pipe->endpoint_id;
	_wait_for_all_events(xhci_device,xhci_pipe->ring);
	spinlock_release_exclusive(&(xhci_device->command_ring->lock));
}



static void _xhci_pipe_transfer_normal(void* ctx,usb_device_t* device,usb_pipe_t* pipe,void* data,u16 length){
	xhci_device_t* xhci_device=ctx;
	xhci_pipe_t* xhci_pipe=pipe;
	spinlock_acquire_exclusive(&(xhci_device->command_ring->lock));
	_enqueue_event(xhci_pipe->ring,data,length,TRB_IOC|TRB_TYPE_TR_NORMAL);
	(xhci_device->doorbell_registers+xhci_pipe->slot)->value=xhci_pipe->endpoint_id;
	_wait_for_all_events(xhci_device,xhci_pipe->ring);
	spinlock_release_exclusive(&(xhci_device->command_ring->lock));
}



static void _xhci_disconnect_port(void* ctx,u16 port){
	panic("_xhci_disconnect_port");
}



static void _xhci_init_device(pci_device_t* device){
	if (device->class!=0x0c||device->subclass!=0x03||device->progif!=0x30){
		return;
	}
	pci_device_enable_memory_access(device);
	pci_device_enable_bus_mastering(device);
	pci_bar_t pci_bar;
	if (!pci_device_get_bar(device,0,&pci_bar)){
		return;
	}
	LOG("Attached XHCI driver to PCI device %x:%x:%x",device->address.bus,device->address.slot,device->address.func);
	xhci_registers_t* registers=(void*)vmm_identity_map(pci_bar.address,sizeof(xhci_registers_t));
	xhci_operational_registers_t* operational_registers=(void*)vmm_identity_map(pci_bar.address+registers->caplength,sizeof(xhci_operational_registers_t));
	if (operational_registers->pagesize!=1){
		WARN("Page size not supported");
		return;
	}
	xhci_device_t* xhci_device=omm_alloc(_xhci_device_allocator);
	xhci_device->registers=registers;
	xhci_device->operational_registers=operational_registers;
	xhci_device->ports=registers->hcsparams1>>24;
	xhci_device->interrupts=(registers->hcsparams1>>8)&0x7ff;
	xhci_device->slots=registers->hcsparams1;
	xhci_device->is_context_64_bytes=!!(registers->hccparams1&0x04);
	xhci_device->port_registers=(void*)vmm_identity_map(pci_bar.address+0x400,xhci_device->ports*sizeof(xhci_port_registers_t));
	xhci_device->doorbell_registers=(void*)vmm_identity_map(pci_bar.address+xhci_device->registers->dboff,xhci_device->ports*sizeof(xhci_doorbell_t));
	xhci_device->interrupt_registers=(void*)vmm_identity_map(pci_bar.address+xhci_device->registers->rtsoff+0x20,xhci_device->interrupts*sizeof(xhci_interrupt_registers_t));
	INFO("Ports: %u, Interrupts: %u, Slots: %u, Context size: %u B",xhci_device->ports,xhci_device->interrupts,xhci_device->slots,(32<<xhci_device->is_context_64_bytes));
	void* data=(void*)(pmm_alloc(pmm_align_up_address(_get_total_memory_size(xhci_device))>>PAGE_SIZE_SHIFT,_xhci_driver_pmm_counter,PMM_MEMORY_HINT_LOW_MEMORY)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	xhci_device->device_context_base_array=data;
	xhci_device->command_ring=_alloc_ring(1);
	xhci_device->event_ring=_alloc_ring(1);
	xhci_device->event_ring_segment=data+_align_size((xhci_device->slots+1)*sizeof(xhci_device_context_base_t));
	if (xhci_device->operational_registers->usbcmd&USBCMD_RS){
		xhci_device->operational_registers->usbcmd&=~USBCMD_RS;
		SPINLOOP(!(xhci_device->operational_registers->usbsts&USBSTS_HCH));
	}
	xhci_device->operational_registers->usbcmd=USBCMD_HCRST;
	SPINLOOP(xhci_device->operational_registers->usbcmd&USBCMD_HCRST);
	SPINLOOP(xhci_device->operational_registers->usbsts&USBSTS_CNR);
	xhci_device->event_ring_segment->address=((u64)(xhci_device->event_ring))-VMM_HIGHER_HALF_ADDRESS_OFFSET;
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
	usb_root_controller_t* root_controller=usb_root_controller_alloc();
	root_controller->device=xhci_device;
	root_controller->pipe_alloc=_xhci_pipe_alloc;
	root_controller->pipe_resize=_xhci_pipe_resize;
	root_controller->pipe_transfer_setup=_xhci_pipe_transfer_setup;
	root_controller->pipe_transfer_normal=_xhci_pipe_transfer_normal;
	usb_controller_t* usb_controller=usb_controller_alloc(root_controller);
	usb_controller->device=xhci_device;
	usb_controller->disconnect=_xhci_disconnect_port;
	usb_device_t* root_hub=usb_device_alloc(usb_controller,NULL,0,USB_DEVICE_SPEED_SUPER);
	for (u16 i=0;i<xhci_device->ports;i++){
		if (!((xhci_device->port_registers+i)->portsc&PORTSC_CCS)){
			continue;
		}
		u8 speed=_reset_port(xhci_device,i);
		if (speed==USB_DEVICE_SPEED_INVALID){
			continue;
		}
		usb_device_alloc(usb_controller,root_hub,i,speed);
	}
}



void xhci_locate_devices(void){
	_xhci_driver_pmm_counter=pmm_alloc_counter("xhci");
	_xhci_input_context_pmm_counter=pmm_alloc_counter("xhci_input_context");
	_xhci_device_allocator=omm_init("xhci_device",sizeof(xhci_device_t),8,1,pmm_alloc_counter("omm_xhci_device"));
	spinlock_init(&(_xhci_device_allocator->lock));
	_xhci_ring_allocator=omm_init("xhci_ring",sizeof(xhci_ring_t),XHCI_RING_SIZE*sizeof(xhci_transfer_block_t),4,pmm_alloc_counter("omm_xhci_ring"));
	spinlock_init(&(_xhci_ring_allocator->lock));
	_xhci_pipe_allocator=omm_init("xhci_pipe",sizeof(xhci_pipe_t),8,2,pmm_alloc_counter("omm_xhci_pipe"));
	spinlock_init(&(_xhci_pipe_allocator->lock));
	HANDLE_FOREACH(pci_device_handle_type){
		pci_device_t* device=handle->object;
		_xhci_init_device(device);
	}
}
