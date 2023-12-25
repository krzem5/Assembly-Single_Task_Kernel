#include <kernel/time/time.h>



u64 syscall_time_get_boot_offset(void){
	return time_boot_offset;
}
