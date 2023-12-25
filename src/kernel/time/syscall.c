#include <kernel/syscall/syscall.h>
#include <kernel/time/time.h>



u64 syscall_time_get_boot_offset(syscall_reg_state_t* regs){
	return time_boot_offset;
}
