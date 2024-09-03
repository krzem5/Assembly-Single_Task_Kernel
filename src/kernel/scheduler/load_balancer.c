#include <kernel/cpu/cpu.h>
#include <kernel/cpu/local.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/load_balancer.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "load_balancer"



static const u32 _scheduler_load_balancer_priority_to_min_queue_index[SCHEDULER_PRIORITY_MAX+1]={
	[SCHEDULER_PRIORITY_REALTIME]=0,
	[SCHEDULER_PRIORITY_HIGH]=2,
	[SCHEDULER_PRIORITY_NORMAL]=8,
	[SCHEDULER_PRIORITY_LOW]=16,
	[SCHEDULER_PRIORITY_BACKGROUND]=24,
};

static const u32 _scheduler_load_balancer_priority_to_max_queue_index[SCHEDULER_PRIORITY_MAX+1]={
	[SCHEDULER_PRIORITY_REALTIME]=1,
	[SCHEDULER_PRIORITY_HIGH]=7,
	[SCHEDULER_PRIORITY_NORMAL]=15,
	[SCHEDULER_PRIORITY_LOW]=23,
	[SCHEDULER_PRIORITY_BACKGROUND]=31,
};

static KERNEL_ATOMIC u64 _scheduler_load_balancer_bitmap=0;
static scheduler_load_balancer_thread_queue_t* KERNEL_INIT_WRITE _scheduler_load_balancer_queues;
static CPU_LOCAL_DATA(scheduler_load_balancer_stats_t,_scheduler_load_balancer_stats);



static KERNEL_INLINE u32 _get_queue_index_offset(const thread_t* thread){
	return (thread->process==process_kernel?0:SCHEDULER_LOAD_BALANCER_QUEUE_COUNT>>1);
}



static KERNEL_INLINE u32 _get_queue_time_us(u32 i){
	return 970+130*(i&((SCHEDULER_LOAD_BALANCER_QUEUE_COUNT>>1)-1));
}



KERNEL_EARLY_INIT(){
	_scheduler_load_balancer_queues=(void*)(pmm_alloc(pmm_align_up_address(SCHEDULER_LOAD_BALANCER_QUEUE_COUNT*sizeof(scheduler_load_balancer_thread_queue_t))>>PAGE_SIZE_SHIFT,pmm_alloc_counter("kernel.scheduler.load_balancer"),0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	for (u32 i=0;i<SCHEDULER_LOAD_BALANCER_QUEUE_COUNT;i++){
		rwlock_init(&((_scheduler_load_balancer_queues+i)->lock));
		(_scheduler_load_balancer_queues+i)->head=NULL;
		(_scheduler_load_balancer_queues+i)->tail=NULL;
	}
}



thread_t* scheduler_load_balancer_get(u32* time_us){
	for (u64 mask=_scheduler_load_balancer_bitmap;mask;mask&=mask-1){
		u64 i=__builtin_ffsll(mask)-1;
		scheduler_load_balancer_thread_queue_t* queue=_scheduler_load_balancer_queues+i;
		if (!rwlock_try_acquire_write(&(queue->lock))){
			continue;
		}
		if (!queue->head){
			__atomic_and_fetch(&_scheduler_load_balancer_bitmap,(~mask)|(mask-1),__ATOMIC_SEQ_CST);
			rwlock_release_write(&(queue->lock));
			continue;
		}
		thread_t* out=queue->head;
		queue->head=out->scheduler_load_balancer_thread_queue_next;
		if (!queue->head){
			queue->tail=NULL;
			__atomic_and_fetch(&_scheduler_load_balancer_bitmap,(~mask)|(mask-1),__ATOMIC_SEQ_CST);
		}
		rwlock_release_write(&(queue->lock));
		((scheduler_load_balancer_stats_t*)CPU_LOCAL(_scheduler_load_balancer_stats))->used_slot_count++;
		*time_us=_get_queue_time_us(i);
		return out;
	}
	((scheduler_load_balancer_stats_t*)CPU_LOCAL(_scheduler_load_balancer_stats))->free_slot_count++;
	*time_us=2500;
	return NULL;
}



void scheduler_load_balancer_add(thread_t* thread){
	if (thread->scheduler_forced_queue_index){
		thread->scheduler_load_balancer_queue_index=thread->scheduler_forced_queue_index;
		goto _skip_dynamic_queue_index;
	}
	if (!thread->scheduler_early_yield&&!thread->scheduler_io_yield&&thread->scheduler_load_balancer_queue_index<SCHEDULER_LOAD_BALANCER_QUEUE_COUNT-1){
		thread->scheduler_load_balancer_queue_index++;
	}
	if (thread->scheduler_io_yield&&thread->scheduler_load_balancer_queue_index){
		thread->scheduler_load_balancer_queue_index--;
	}
	thread->scheduler_early_yield=0;
	thread->scheduler_io_yield=0;
	u32 min=_get_queue_index_offset(thread)+_scheduler_load_balancer_priority_to_min_queue_index[thread->priority];
	u32 max=_get_queue_index_offset(thread)+_scheduler_load_balancer_priority_to_max_queue_index[thread->priority];
	if (thread->scheduler_load_balancer_queue_index<min){
		thread->scheduler_load_balancer_queue_index=min;
	}
	else if (thread->scheduler_load_balancer_queue_index>max){
		thread->scheduler_load_balancer_queue_index=max;
	}
_skip_dynamic_queue_index:
	scheduler_load_balancer_thread_queue_t* queue=_scheduler_load_balancer_queues+thread->scheduler_load_balancer_queue_index;
	rwlock_acquire_write(&(queue->lock));
	thread->scheduler_load_balancer_thread_queue_next=NULL;
	if (queue->tail){
		queue->tail->scheduler_load_balancer_thread_queue_next=thread;
	}
	else{
		queue->head=thread;
	}
	queue->tail=thread;
	__atomic_or_fetch(&_scheduler_load_balancer_bitmap,1ull<<thread->scheduler_load_balancer_queue_index,__ATOMIC_SEQ_CST);
	rwlock_release_write(&(queue->lock));
}



void scheduler_load_balancer_remove(thread_t* thread){
	panic("scheduler_load_balancer_remove");
}



KERNEL_PUBLIC const scheduler_load_balancer_stats_t* scheduler_load_balancer_get_stats(u16 cpu_index){
	return (cpu_index>=cpu_count?NULL:_scheduler_load_balancer_stats+cpu_index);
}
