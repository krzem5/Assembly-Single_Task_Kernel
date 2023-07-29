#include <kernel/log/log.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "isr"



void _isr_handler(u64* vars,u8 index){
	WARN("ISR %u:",index);
	WARN("error=%p",vars[0]);
	WARN("rax=%p",vars[1]);
	WARN("rbx=%p",vars[2]);
	WARN("rcx=%p",vars[3]);
	WARN("rdx=%p",vars[4]);
	WARN("rsi=%p",vars[5]);
	WARN("rdi=%p",vars[6]);
	WARN("rbp=%p",vars[7]);
	WARN("r8=%p",vars[8]);
	WARN("r9=%p",vars[9]);
	WARN("r10=%p",vars[10]);
	WARN("r11=%p",vars[11]);
	WARN("r12=%p",vars[12]);
	WARN("r13=%p",vars[13]);
	WARN("r14=%p",vars[14]);
	WARN("r15=%p",vars[15]);
	for (;;);
}



void _isr_kernel(u8 index){
	ERROR("Received ISR from inside the kernel: %u",index);
	for (;;);
}
