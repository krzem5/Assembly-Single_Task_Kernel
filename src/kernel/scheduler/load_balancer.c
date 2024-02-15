#include <kernel/cpu/cpu.h>
#include <kernel/cpu/local.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/scheduler/load_balancer.h>
#include <kernel/types.h>
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

static u64 _scheduler_load_balancer_bitmap=0;
static scheduler_load_balancer_thread_queue_t* KERNEL_INIT_WRITE _scheduler_load_balancer_queues;
static CPU_LOCAL_DATA(scheduler_load_balancer_stats_t,_scheduler_load_balancer_stats);



static u32 _get_queue_index_offset(const thread_t* thread){
	return (thread->process->handle.rb_node.key?SCHEDULER_LOAD_BALANCER_QUEUE_COUNT>>1:0);
}



static u32 _get_queue_time_us(u32 i){
	return 968+((i&((SCHEDULER_LOAD_BALANCER_QUEUE_COUNT>>1)-1))<<7);
}



KERNEL_EARLY_INIT(){
	LOG("Initializing scheduler load balancer...");
	_scheduler_load_balancer_queues=(void*)(pmm_alloc(pmm_align_up_address(SCHEDULER_LOAD_BALANCER_QUEUE_COUNT*sizeof(scheduler_load_balancer_thread_queue_t))>>PAGE_SIZE_SHIFT,pmm_alloc_counter("scheduler_load_balancer"),0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	for (u32 i=0;i<SCHEDULER_LOAD_BALANCER_QUEUE_COUNT;i++){
		spinlock_init(&((_scheduler_load_balancer_queues+i)->lock));
		(_scheduler_load_balancer_queues+i)->head=NULL;
		(_scheduler_load_balancer_queues+i)->tail=NULL;
	}
}



thread_t* scheduler_load_balancer_get(u32* time_us){
	while (_scheduler_load_balancer_bitmap){
		u32 i=__builtin_ffsll(_scheduler_load_balancer_bitmap)-1;
		scheduler_load_balancer_thread_queue_t* queue=_scheduler_load_balancer_queues+i;
		spinlock_acquire_exclusive(&(queue->lock));
		if (!queue->head){
			spinlock_release_exclusive(&(queue->lock));
			continue;
		}
		thread_t* out=queue->head;
		queue->head=out->scheduler_load_balancer_thread_queue_next;
		if (!queue->head){
			queue->tail=NULL;
			_scheduler_load_balancer_bitmap&=~(1ull<<i);
		}
		spinlock_release_exclusive(&(queue->lock));
		((scheduler_load_balancer_stats_t*)CPU_LOCAL(_scheduler_load_balancer_stats))->used_slot_count++;
		*time_us=_get_queue_time_us(i);
		return out;
	}
	((scheduler_load_balancer_stats_t*)CPU_LOCAL(_scheduler_load_balancer_stats))->free_slot_count++;
	*time_us=2500;
	return NULL;
}



void scheduler_load_balancer_add(thread_t* thread){
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
	scheduler_load_balancer_thread_queue_t* queue=_scheduler_load_balancer_queues+thread->scheduler_load_balancer_queue_index;
	spinlock_acquire_exclusive(&(queue->lock));
	thread->scheduler_load_balancer_thread_queue_next=NULL;
	if (queue->tail){
		queue->tail->scheduler_load_balancer_thread_queue_next=thread;
	}
	else{
		queue->head=thread;
	}
	queue->tail=thread;
	spinlock_release_exclusive(&(queue->lock));
	_scheduler_load_balancer_bitmap|=1ull<<thread->scheduler_load_balancer_queue_index;
}



KERNEL_PUBLIC const scheduler_load_balancer_stats_t* scheduler_load_balancer_get_stats(u16 cpu_index){
	return (cpu_index>=cpu_count?NULL:_scheduler_load_balancer_stats+cpu_index);
}
