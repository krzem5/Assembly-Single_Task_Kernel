#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/ring/ring.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "ring"



static omm_allocator_t* _ring_allocator=NULL;
static pmm_counter_descriptor_t* _ring_buffer_pmm_counter=NULL;



KERNEL_PUBLIC ring_t* ring_init(u32 capacity){
	if (capacity&(capacity-1)){
		ERROR("Ring capacity must be a power of 2");
		return NULL;
	}
	if (!_ring_allocator){
		_ring_allocator=omm_init("ring",sizeof(ring_t),8,4,pmm_alloc_counter("omm_ring"));
		spinlock_init(&(_ring_allocator->lock));
	}
	if (!_ring_buffer_pmm_counter){
		_ring_buffer_pmm_counter=pmm_alloc_counter("ring_buffer");
	}
	ring_t* out=omm_alloc(_ring_allocator);
	out->data=(void*)(pmm_alloc(pmm_align_up_address(capacity*sizeof(void*))>>PAGE_SIZE_SHIFT,_ring_buffer_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	out->capacity=capacity;
	out->read_index=0;
	out->read_count=0;
	out->write_index=0;
	out->write_count=capacity;
	spinlock_init(&(out->read_lock));
	spinlock_init(&(out->write_lock));
	out->read_event=event_new();
	out->write_event=event_new();
	return out;
}



KERNEL_PUBLIC void ring_deinit(ring_t* ring);



KERNEL_PUBLIC _Bool ring_push(ring_t* ring,void* item,_Bool wait);



KERNEL_PUBLIC void* ring_pop(ring_t* ring,_Bool wait);
