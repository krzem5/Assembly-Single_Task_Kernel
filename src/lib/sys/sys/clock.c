#include <sys/syscall.h>
#include <sys/types.h>



static u64 _sys_clock_conversion_factor=0;
static u64 _sys_clock_conversion_shift=0;
static u64 _sys_clock_cpu_frequency=0;



static inline u64 _get_ticks(void){
	u32 low;
	u32 high;
	asm volatile("rdtsc":"=a"(low),"=d"(high));
	return (((u64)high)<<32)|low;
}



SYS_PUBLIC void sys_clock_init(void){
	u64 buffer[3];
	_syscall_clock_get_converion(buffer);
	_sys_clock_conversion_factor=buffer[0];
	_sys_clock_conversion_shift=buffer[1];
	_sys_clock_cpu_frequency=buffer[2];
}



SYS_PUBLIC u64 sys_clock_get_ticks(void){
	return _get_ticks();
}



SYS_PUBLIC u64 sys_clock_get_time(void){
	return (_get_ticks()*_sys_clock_conversion_factor)>>_sys_clock_conversion_shift;
}



SYS_PUBLIC u64 sys_clock_ticks_to_time(u64 ticks){
	return (ticks*_sys_clock_conversion_factor)>>_sys_clock_conversion_shift;
}



SYS_PUBLIC u64 sys_clock_get_frequency(void){
	return _sys_clock_cpu_frequency;
}
