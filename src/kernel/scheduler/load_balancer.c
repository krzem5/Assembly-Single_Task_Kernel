#include <kernel/cpu/cpu.h>
#include <kernel/cpu/local.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/scheduler/cpu_mask.h>
#include <kernel/scheduler/load_balancer.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "load_balancer"



#define DECLARE_QUEUE_ORDER(a,b,c,d,e) ((a)|((b)<<SCHEDULER_PRIORITY_SHIFT)|((c)<<(2*SCHEDULER_PRIORITY_SHIFT))|((d)<<(3*SCHEDULER_PRIORITY_SHIFT))|((e)<<(4*SCHEDULER_PRIORITY_SHIFT)))



static CPU_LOCAL_DATA(scheduler_load_balancer_data_t,_scheduler_load_balancer_data);
static CPU_LOCAL_DATA(scheduler_load_balancer_data_t*,_scheduler_load_balancer_priority_queue);
static CPU_LOCAL_DATA(scheduler_load_balancer_group_t,_scheduler_load_balancer_groups);
static scheduler_load_balancer_t _scheduler_load_balancer;



static void _thread_queue_init(scheduler_load_balancer_thread_queue_t* queue){
	lock_init(&(queue->lock));
	queue->head=NULL;
	queue->tail=NULL;
}



static u32 _get_queue_order(scheduler_load_balancer_data_t* lb_data){
	lb_data->queue_access_timing++;
	if (!(lb_data->queue_access_timing&15)){
		return DECLARE_QUEUE_ORDER(
			SCHEDULER_PRIORITY_REALTIME,
			SCHEDULER_PRIORITY_LOW,
			SCHEDULER_PRIORITY_HIGH,
			SCHEDULER_PRIORITY_NORMAL,
			SCHEDULER_PRIORITY_BACKGROUND
		);
	}
	else if (!(lb_data->queue_access_timing&3)){
		return DECLARE_QUEUE_ORDER(
			SCHEDULER_PRIORITY_REALTIME,
			SCHEDULER_PRIORITY_NORMAL,
			SCHEDULER_PRIORITY_HIGH,
			SCHEDULER_PRIORITY_LOW,
			SCHEDULER_PRIORITY_BACKGROUND
		);
	}
	else{
		return DECLARE_QUEUE_ORDER(
			SCHEDULER_PRIORITY_REALTIME,
			SCHEDULER_PRIORITY_HIGH,
			SCHEDULER_PRIORITY_NORMAL,
			SCHEDULER_PRIORITY_LOW,
			SCHEDULER_PRIORITY_BACKGROUND
		);
	}
}



static thread_t* _try_pop_from_queue(scheduler_load_balancer_thread_queue_t* queue){
	if (!queue->head){
		return NULL;
	}
	lock_acquire_exclusive(&(queue->lock));
	if (!queue->head){
		lock_release_exclusive(&(queue->lock));
		return NULL;
	}
	thread_t* out=queue->head;
	queue->head=out->scheduler_load_balancer_thread_queue_next;
	if (queue->tail==out){
		queue->tail=NULL;
	}
	lock_release_exclusive(&(queue->lock));
	return out;
}



void scheduler_load_balancer_init(void){
	LOG("Initializing scheduler load balancer...");
	lock_init(&(_scheduler_load_balancer.lock));
	_scheduler_load_balancer.free_group=NULL;
	_scheduler_load_balancer_groups->length=cpu_count;
	_scheduler_load_balancer_groups->end=cpu_count-1;
	for (u16 i=0;i<cpu_count;i++){
		scheduler_load_balancer_data_t* lb_data=_scheduler_load_balancer_data+i;
		_scheduler_load_balancer_priority_queue[i]=lb_data;
		lb_data->stats.added_thread_count=0;
		lb_data->stats.free_slot_count=0;
		lb_data->stats.used_slot_count=0;
		lb_data->group=_scheduler_load_balancer_groups;
		for (u8 j=0;j<SCHEDULER_LOAD_BALANCER_THREAD_QUEUE_COUNT;j++){
			_thread_queue_init(lb_data->queues+j);
		}
		lb_data->cpu_index=i;
		lb_data->queue_access_timing=0;
		if (!i){
			continue;
		}
		(_scheduler_load_balancer_groups+i)->next_group=_scheduler_load_balancer.free_group;
		_scheduler_load_balancer.free_group=_scheduler_load_balancer_groups+i;
	}
}



thread_t* scheduler_load_balancer_get(void){
	scheduler_load_balancer_data_t* lb_data=CPU_LOCAL(_scheduler_load_balancer_data);
	u32 queue_order=_get_queue_order(lb_data);
	for (u8 i=0;i<SCHEDULER_LOAD_BALANCER_THREAD_QUEUE_COUNT;i++){
		thread_t* out=_try_pop_from_queue(lb_data->queues+(queue_order&((1<<SCHEDULER_PRIORITY_SHIFT)-1)));
		if (out){
			lb_data->stats.used_slot_count++;
			return out;
		}
		queue_order>>=SCHEDULER_PRIORITY_SHIFT;
	}
	lb_data->stats.free_slot_count++;
	return NULL;
}



void scheduler_load_balancer_add(thread_t* thread){
	lock_acquire_exclusive(&(_scheduler_load_balancer.lock));
	u16 i=0;
	scheduler_load_balancer_data_t* out;
	while (1){
		out=_scheduler_load_balancer_priority_queue[i];
		if (i==cpu_count-1||(thread->cpu_mask->bitmap[out->cpu_index>>6]&(1ull<<(out->cpu_index&63)))){
			break;
		}
		i++;
	}
	out->stats.added_thread_count++;
	u16 j=out->group->end;
	_scheduler_load_balancer_priority_queue[i]=_scheduler_load_balancer_priority_queue[j];
	_scheduler_load_balancer_priority_queue[j]=out;
	out->group->end--;
	out->group->length--;
	if (!out->group->length){
		out->group->next_group=_scheduler_load_balancer.free_group;
		_scheduler_load_balancer.free_group=out->group;
	}
	if (j==cpu_count-1||_scheduler_load_balancer_priority_queue[j+1]->stats.added_thread_count>out->stats.added_thread_count){
		out->group=_scheduler_load_balancer.free_group;
		_scheduler_load_balancer.free_group=_scheduler_load_balancer.free_group->next_group;
		out->group->length=1;
		out->group->end=j;
	}
	else{
		out->group=_scheduler_load_balancer_priority_queue[j+1]->group;
		out->group->length++;
	}
	lock_release_exclusive(&(_scheduler_load_balancer.lock));
	scheduler_load_balancer_thread_queue_t* queue=out->queues+thread->priority;
	lock_acquire_exclusive(&(queue->lock));
	if (queue->tail){
		queue->tail->scheduler_load_balancer_thread_queue_next=thread;
	}
	else{
		queue->head=thread;
	}
	queue->tail=thread;
	thread->scheduler_load_balancer_thread_queue_next=NULL;
	lock_release_exclusive(&(queue->lock));
}



_Bool scheduler_load_balancer_get_stats(u16 cpu_index,scheduler_load_balancer_stats_t* out){
	if (cpu_index>=cpu_count){
		return 0;
	}
	*out=(_scheduler_load_balancer_data+cpu_index)->stats;
	return 1;
}
