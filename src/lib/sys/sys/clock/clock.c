#include <sys/clock/clock.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>



static u64 _sys_clock_cpu_frequency=0;

u64 _sys_clock_conversion_factor=0;
u32 _sys_clock_conversion_shift=0;



void __sys_clock_init(void){
	u64 buffer[3];
	_sys_syscall_clock_get_converion(buffer);
	_sys_clock_conversion_factor=buffer[0];
	_sys_clock_conversion_shift=buffer[1];
	_sys_clock_cpu_frequency=buffer[2];
}



SYS_PUBLIC u64 sys_clock_get_ticks(void){
	u32 low;
	u32 high;
	asm volatile("rdtsc":"=a"(low),"=d"(high));
	return (((u64)high)<<32)|low;
}



SYS_PUBLIC double sys_clock_get_time(void){
	return sys_clock_get_time_ns()/1000000000.0;
}



SYS_PUBLIC u64 sys_clock_get_time_ns(void){
	return sys_clock_convert_ticks_to_time_ns(sys_clock_get_ticks());
}



SYS_PUBLIC double sys_clock_convert_ticks_to_time(u64 ticks){
	return sys_clock_convert_ticks_to_time_ns(ticks)/1000000000.0;
}



SYS_PUBLIC u64 sys_clock_get_frequency(void){
	return _sys_clock_cpu_frequency;
}
