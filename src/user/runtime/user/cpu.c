#include <user/syscall.h>
#include <user/types.h>



u32 cpu_count;
u32 cpu_bsp_id;



void cpu_init(void){
	u64 data=_syscall_cpu_core_count();
	cpu_count=data;
	cpu_bsp_id=data>>32;
}



void cpu_core_start(u32 core,void* func,void* arg){
	_syscall_cpu_core_start(core,func,arg);
}



void cpu_core_stop(void){
	_syscall_cpu_core_stop();
}
