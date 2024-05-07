#include <kernel/lock/bitlock.h>
#include <kernel/types.h>
#include <kernel/util/spinloop.h>



KERNEL_PUBLIC void bitlock_init(u32* field,u32 bit){
	*field&=~(1<<bit);
}



KERNEL_PUBLIC void bitlock_acquire(u32* field,u32 bit){
	u32 mask=1<<bit;
	while (__builtin_expect(__atomic_fetch_or(field,mask,__ATOMIC_SEQ_CST)&mask,0)){
		__pause();
	}
}



KERNEL_PUBLIC bool bitlock_try_acquire(u32* field,u32 bit){
	u32 mask=1<<bit;
	return !(__atomic_fetch_or(field,mask,__ATOMIC_SEQ_CST)&mask);
}



KERNEL_PUBLIC void bitlock_release(u32* field,u32 bit){
	__atomic_fetch_and(field,~(1<<bit),__ATOMIC_SEQ_CST);
}



KERNEL_PUBLIC bool bitlock_is_held(u32* field,u32 bit){
	return ((*field)>>bit)&1;
}
