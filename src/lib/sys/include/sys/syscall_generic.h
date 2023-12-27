#ifndef _SYS_SYSCALL_GENERIC_H_
#define _SYS_SYSCALL_GENERIC_H_ 1
#include <sys/types.h>



static inline u64 _syscall0(u64 rax){
	u64 out;
	asm volatile("syscall":"=a"(out):"a"(rax):"rcx","r11","memory");
	return out;
}



static inline u64 _syscall1(u64 rax,u64 arg0){
	u64 out;
	asm volatile("syscall":"=a"(out):"a"(rax),"D"(arg0):"rcx","r11","memory");
	return out;
}



static inline u64 _syscall2(u64 rax,u64 arg0,u64 arg1){
	u64 out;
	asm volatile("syscall":"=a"(out):"a"(rax),"D"(arg0),"S"(arg1):"rcx","r11","memory");
	return out;
}



static inline u64 _syscall3(u64 rax,u64 arg0,u64 arg1,u64 arg2){
	u64 out;
	asm volatile("syscall":"=a"(out):"a"(rax),"D"(arg0),"S"(arg1),"d"(arg2):"rcx","r11","memory");
	return out;
}



static inline u64 _syscall4(u64 rax,u64 arg0,u64 arg1,u64 arg2,u64 arg3){
	register u64 arg3_reg asm("r10")=arg3;
	u64 out;
	asm volatile("syscall":"=a"(out):"a"(rax),"D"(arg0),"S"(arg1),"d"(arg2),"r"(arg3_reg):"rcx","r11","memory");
	return out;
}



static inline u64 _syscall5(u64 rax,u64 arg0,u64 arg1,u64 arg2,u64 arg3,u64 arg4){
	register u64 arg3_reg asm("r10")=arg3;
	register u64 arg4_reg asm("r8")=arg4;
	u64 out;
	asm volatile("syscall":"=a"(out):"a"(rax),"D"(arg0),"S"(arg1),"d"(arg2),"r"(arg3_reg),"r"(arg4_reg):"rcx","r11","memory");
	return out;
}



static inline u64 _syscall6(u64 rax,u64 arg0,u64 arg1,u64 arg2,u64 arg3,u64 arg4,u64 arg5){
	register u64 arg3_reg asm("r10")=arg3;
	register u64 arg4_reg asm("r8")=arg4;
	register u64 arg5_reg asm("r9")=arg5;
	u64 out;
	asm volatile("syscall":"=a"(out):"a"(rax),"D"(arg0),"S"(arg1),"d"(arg2),"r"(arg3_reg),"r"(arg4_reg),"r"(arg5_reg):"rcx","r11","memory");
	return out;
}



#endif
