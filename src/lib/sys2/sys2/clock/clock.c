#include <sys2/clock/clock.h>
#include <sys2/syscall/kernel_syscalls.h>
#include <sys2/types.h>



static u64 _sys2_clock_conversion_factor=0;
static u64 _sys2_clock_conversion_shift=0;
static u64 _sys2_clock_cpu_frequency=0;



void __sys2_clock_init(void){
	u64 buffer[3];
	_sys2_syscall_clock_get_converion(buffer);
	_sys2_clock_conversion_factor=buffer[0];
	_sys2_clock_conversion_shift=buffer[1];
	_sys2_clock_cpu_frequency=buffer[2];
}



SYS2_PUBLIC u64 sys2_clock_get_ticks(void){
	u32 low;
	u32 high;
	asm volatile("rdtsc":"=a"(low),"=d"(high));
	return (((u64)high)<<32)|low;
}



SYS2_PUBLIC double sys2_clock_get_time(void){
	return sys2_clock_get_time_ns()/1000000000.0;
}



SYS2_PUBLIC u64 sys2_clock_get_time_ns(void){
	return (sys2_clock_get_ticks()*_sys2_clock_conversion_factor)>>_sys2_clock_conversion_shift;
}



SYS2_PUBLIC double sys2_clock_convert_ticks_to_time(u64 ticks){
	return sys2_clock_convert_ticks_to_time_ns(ticks)/1000000000.0;
}



SYS2_PUBLIC u64 sys2_clock_convert_ticks_to_time_ns(u64 ticks){
	return (ticks*_sys2_clock_conversion_factor)>>_sys2_clock_conversion_shift;
}



SYS2_PUBLIC u64 sys2_clock_get_frequency(void){
	return _sys2_clock_cpu_frequency;
}
