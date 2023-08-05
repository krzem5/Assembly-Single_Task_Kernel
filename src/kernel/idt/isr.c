#include <kernel/log/log.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "isr"



void _isr_handler(u64* vars,u8 index,u64* extra_data){
	WARN("ISR %u:",index);
	WARN("error=%p",vars[0]);
	WARN("cr0=%p",vars[1]);
	WARN("cr2=%p",vars[2]);
	WARN("cr3=%p",vars[3]);
	WARN("cr4=%p",vars[4]);
	WARN("rax=%p",vars[5]);
	WARN("rbx=%p",vars[6]);
	WARN("rcx=%p",vars[7]);
	WARN("rdx=%p",vars[8]);
	WARN("rsi=%p",vars[9]);
	WARN("rdi=%p",vars[10]);
	WARN("rbp=%p",vars[11]);
	WARN("r8=%p",vars[12]);
	WARN("r9=%p",vars[13]);
	WARN("r10=%p",vars[14]);
	WARN("r11=%p",vars[15]);
	WARN("r12=%p",vars[16]);
	WARN("r13=%p",vars[17]);
	WARN("r14=%p",vars[18]);
	WARN("r15=%p",vars[19]);
	WARN("rip=%p",extra_data[0]);
	WARN("rflags=%p",extra_data[2]);
	WARN("rsp=%p",extra_data[3]);
	for (;;);
}



void _isr_handler_inside_kernel(u8 index){
	ERROR("ISR %u inside kernel",index);
	for (;;);
}
