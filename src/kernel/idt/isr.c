#include <kernel/log/log.h>
#include <kernel/types.h>



void _isr_handler(u64* vars,u8 index){
	WARN("ISR %u:",index);
	WARN("rax=%p",vars[0]);
	WARN("rbx=%p",vars[1]);
	WARN("rcx=%p",vars[2]);
	WARN("rdx=%p",vars[3]);
	WARN("rsi=%p",vars[4]);
	WARN("rdi=%p",vars[5]);
	WARN("rbp=%p",vars[6]);
	WARN("r8=%p",vars[7]);
	WARN("r9=%p",vars[8]);
	WARN("r10=%p",vars[9]);
	WARN("r11=%p",vars[10]);
	WARN("r12=%p",vars[11]);
	WARN("r13=%p",vars[12]);
	WARN("r14=%p",vars[13]);
	WARN("r15=%p",vars[14]);
	WARN("[0]=%p",vars[15]);
	WARN("[1]=%p",vars[16]);
	WARN("[2]=%p",vars[17]);
	WARN("[3]=%p",vars[18]);
	WARN("[4]=%p",vars[19]);
	for (;;);
}



void _isr_kernel(u8 index){
	ERROR("Received ISR from inside the kernel: %u",index);
	for (;;);
}
