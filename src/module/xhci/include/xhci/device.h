#ifndef _XHCI_DEVICE_H_
#define _XHCI_DEVICE_H_ 1
#include <kernel/lock/rwlock.h>
#include <kernel/types.h>
#include <xhci/registers.h>



#define XHCI_RING_SIZE 16



typedef struct _XHCI_RING{
	xhci_transfer_block_t ring[XHCI_RING_SIZE];
	xhci_transfer_block_t event;
	rwlock_t lock;
	u32 eidx;
	u32 nidx;
	u32 cs;
} xhci_ring_t;



typedef struct _XHCI_DEVICE{
	xhci_registers_t* registers;
	xhci_operational_registers_t* operational_registers;
	xhci_port_registers_t* port_registers;
	xhci_doorbell_t* doorbell_registers;
	xhci_interrupt_registers_t* interrupt_registers;
	xhci_device_context_base_t* device_context_base_array;
	xhci_ring_t* command_ring;
	xhci_ring_t* event_ring;
	xhci_event_ring_segment_t* event_ring_segment;
	u8 ports;
	u16 interrupts;
	u8 slots;
	bool is_context_64_bytes;
} xhci_device_t;



typedef struct _XHCI_PIPE{
	xhci_ring_t* ring;
	u32 slot;
	u32 endpoint_id;
} xhci_pipe_t;



#endif
