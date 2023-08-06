#include <user/syscall.h>
#include <user/types.h>
#include <user/io.h>



u32 cpu_count;
u32 cpu_bsp_id;



void cpu_init(void){
	u64 data=_syscall_cpu_core_count();
	cpu_count=data;
	cpu_bsp_id=data>>32;
}
