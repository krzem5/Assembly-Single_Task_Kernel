#include <kernel/clock/clock.h>
#include <kernel/syscall/syscall.h>



u64 syscall_clock_get_converion(u64* buffer){
	buffer[0]=clock_conversion_factor;
	buffer[1]=clock_conversion_shift;
	buffer[2]=clock_cpu_frequency;
	return 0;
}
