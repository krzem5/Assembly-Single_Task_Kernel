#ifndef _KERNEL_RING_RING_H_
#define _KERNEL_RING_RING_H_ 1
#include <kernel/lock/spinlock.h>
#include <kernel/mp/event.h>
#include <kernel/types.h>



#define RING_FLAG_HAS_SPACE 1
#define RING_FLAG_HAS_DATA 2



typedef struct _RING{
	void** data;
	u16 mask;
	u16 head;
	u16 tail;
	u16 flags;
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
