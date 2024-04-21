#include <kernel/clock/clock.h>
#include <kernel/error/error.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "clock"



typedef struct _CLOCK_SOURCE_WRAPPER{
	const clock_source_t* source;
	struct _CLOCK_SOURCE_WRAPPER* next;
} clock_source_wrapper_t;



static omm_allocator_t* _clock_source_wrapper_allocator=NULL;
static clock_source_wrapper_t* _clock_source_head=NULL;
static clock_source_wrapper_t* _clock_source_tail=NULL;

u64 KERNEL_INIT_WRITE clock_cpu_frequency;
u64 KERNEL_INIT_WRITE clock_conversion_factor;
u32 KERNEL_INIT_WRITE clock_conversion_shift;



KERNEL_EARLY_INIT(){
	LOG("Initializing clock source...");
	INFO("Checking stable sources...");
	clock_cpu_frequency=0;
	for (clock_source_wrapper_t* wrapper=_clock_source_head;wrapper;wrapper=wrapper->next){
		if (*(wrapper->source->is_stable)){
			clock_cpu_frequency=wrapper->source->calibration_callback();
			if (clock_cpu_frequency){
				LOG("Selected clock source '%s'",wrapper->source->name);
				goto _clock_source_found;
			}
		}
	}
	INFO("Checking unstable sources...");
	for (clock_source_wrapper_t* wrapper=_clock_source_head;wrapper;wrapper=wrapper->next){
		if (!*(wrapper->source->is_stable)){
			clock_cpu_frequency=wrapper->source->calibration_callback();
			if (clock_cpu_frequency){
				LOG("Selected clock source '%s'",wrapper->source->name);
				break;
			}
		}
	}
_clock_source_found:
	LOG("CPU clock frequency: %lu Hz",clock_cpu_frequency);
	INFO("Calculating clock frequency conversion factor...");
	for (clock_conversion_shift=32;clock_conversion_shift;clock_conversion_shift--){
		clock_conversion_factor=((1000000000ull<<clock_conversion_shift)+(clock_cpu_frequency>>1))/clock_cpu_frequency;
		if (!(clock_conversion_factor>>32)){
			return;
		}
	}
	panic("Unable to calculate clock frequency conversion factor");
}



void clock_add_source(const clock_source_t* source){
	LOG("Registering clock source '%s'...",source->name);
	if (!_clock_source_wrapper_allocator){
		_clock_source_wrapper_allocator=omm_init("clock_source_wrapper",sizeof(clock_source_wrapper_t),8,1);
		spinlock_init(&(_clock_source_wrapper_allocator->lock));
	}
	clock_source_wrapper_t* wrapper=omm_alloc(_clock_source_wrapper_allocator);
	wrapper->source=source;
	wrapper->next=NULL;
	if (_clock_source_tail){
		_clock_source_tail->next=wrapper;
	}
	else{
		_clock_source_head=wrapper;
	}
	_clock_source_tail=wrapper;
}



error_t syscall_clock_get_converion(KERNEL_USER_POINTER u64* buffer){
	buffer[0]=clock_conversion_factor;
	buffer[1]=clock_conversion_shift;
	buffer[2]=clock_cpu_frequency;
	return ERROR_OK;
}
