#ifndef _KERNEL_RING_RING_H_
#define _KERNEL_RING_RING_H_ 1
#include <kernel/lock/spinlock.h>
#include <kernel/mp/event.h>
#include <kernel/types.h>



typedef struct _RING{
	void** buffer;
	u32 capacity;
	u32 read_index;
	KERNEL_ATOMIC u32 read_count;
	u32 write_index;
	KERNEL_ATOMIC u32 write_count;
	spinlock_t read_lock;
	spinlock_t write_lock;
	event_t* read_event;
	event_t* write_event;
} ring_t;



ring_t* ring_init(u32 capacity);



void ring_deinit(ring_t* ring);



_Bool ring_push(ring_t* ring,void* item,_Bool wait);



void* ring_pop(ring_t* ring,_Bool wait);



#endif
