#include <kernel/clock/clock.h>
#include <kernel/error/error.h>
#include <kernel/io/io.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "clock"



#define PIT_FREQUENCY 1193182



static _Bool KERNEL_INIT_WRITE _tsc_pit_clock_source_is_stable=0;



static KERNEL_INLINE u64 KERNEL_NOCOVERAGE _wait_for_pit(u8 high){
	for (u32 i=0;i<500000;i++){
		io_port_in8(0x42);
		if (io_port_in8(0x42)<high){
			if (i<10){
				return 0;
			}
			u32 low;
			u32 high;
			asm volatile("rdtsc":"=a"(low),"=d"(high));
			return (((u64)high)<<32)|low;
		}
	}
	return 0;
}



static u64 _calibration_callback(void){
	while (1){
		io_port_out8(0x61,(io_port_in8(0x61)&0xfd)|0x01);
		io_port_out8(0x43,0xb0);
		io_port_out8(0x42,0xff);
		io_port_out8(0x42,0xff);
		io_port_in8(0x42);
		io_port_in8(0x42);
		u64 start_tsc=_wait_for_pit(255);
		if (!start_tsc){
			continue;
		}
		u16 start_pit=io_port_in8(0x42)|(io_port_in8(0x42)<<8);
		for (u8 i=128;i<255;i++){
			u64 end_tsc=_wait_for_pit(255-i);
			if (!end_tsc){
				continue;
			}
			u16 end_pit=io_port_in8(0x42)|(io_port_in8(0x42)<<8);
			if ((end_pit>>8)!=254-i){
				continue;
			}
			return (end_tsc-start_tsc)*PIT_FREQUENCY/(start_pit-end_pit);
		}
	}
}



static const clock_source_t _tsc_pit_clock_source={
	"TSC/PIT",
	_calibration_callback,
	&_tsc_pit_clock_source_is_stable
};



KERNEL_EARLY_EARLY_INIT(){
	clock_add_source(&_tsc_pit_clock_source);
}



/*****************************************************************************/



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
	if (!_clock_source_wrapper_allocator){
		_clock_source_wrapper_allocator=omm_init("clock_source_wrapper",sizeof(clock_source_wrapper_t),8,1,pmm_alloc_counter("clock_source_wrapper"));
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
