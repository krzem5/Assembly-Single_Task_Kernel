#ifndef _KERNEL_IO_IO_H_
#define _KERNEL_IO_IO_H_ 1
#include <kernel/types.h>



static inline u8 KERNEL_CORE_CODE KERNEL_NOCOVERAGE io_port_in8(u16 port){
	u8 out;
	asm volatile("in %1, %0":"=a"(out):"Nd"(port):"memory");
	return out;
}



static inline u16 KERNEL_CORE_CODE KERNEL_NOCOVERAGE io_port_in16(u16 port){
	u16 out;
	asm volatile("inw %1, %0":"=a"(out):"Nd"(port):"memory");
	return out;
}



static inline u32 KERNEL_CORE_CODE KERNEL_NOCOVERAGE io_port_in32(u16 port){
	u32 out;
	asm volatile("inl %1, %0":"=a"(out):"Nd"(port):"memory");
	return out;
}



static inline void KERNEL_CORE_CODE KERNEL_NOCOVERAGE io_port_out8(u16 port,u8 value){
	asm volatile("outb %0, %1"::"a"(value),"Nd"(port):"memory");
}



static inline void KERNEL_CORE_CODE KERNEL_NOCOVERAGE io_port_out16(u16 port,u16 value){
	asm volatile("outw %0, %1"::"a"(value),"Nd"(port):"memory");
}



static inline void KERNEL_CORE_CODE KERNEL_NOCOVERAGE io_port_out32(u16 port,u32 value){
	asm volatile("outl %0, %1"::"a"(value),"Nd"(port):"memory");
}



#endif
