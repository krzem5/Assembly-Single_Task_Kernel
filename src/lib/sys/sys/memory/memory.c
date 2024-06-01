#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/memory/memory.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>



SYS_PUBLIC u64 sys_memory_map(u64 length,u32 flags,sys_fd_t fd){
	return _sys_syscall_memory_map(length,flags,fd);
}



SYS_PUBLIC sys_error_t sys_memory_change_flags(void* address,u64 length,u32 flags){
	return _sys_syscall_memory_change_flags(address,length,flags);
}



SYS_PUBLIC sys_error_t sys_memory_get_size(void* address){
	return _sys_syscall_memory_get_size(address);
}



SYS_PUBLIC sys_error_t sys_memory_unmap(void* address,u64 length){
	return _sys_syscall_memory_unmap(address,length);
}



SYS_PUBLIC void sys_memory_copy(const void* src,void* dst,u64 length){
	const u8* src_ptr=src;
	u8* dst_ptr=dst;
	for (u64 i=0;i<length;i++){
		dst_ptr[i]=src_ptr[i];
	}
}



SYS_PUBLIC void sys_memory_exchange(void* a,void* b,u64 length){
	u8* a_ptr=a;
	u8* b_ptr=b;
	for (u64 i=0;i<length;i++){
		u8 tmp=a_ptr[i];
		a_ptr[i]=b_ptr[i];
		b_ptr[i]=tmp;
	}
}



SYS_PUBLIC s32 sys_memory_compare(const void* a,const void* b,u64 length){
	const u8* ptr_a=a;
	const u8* ptr_b=b;
	for (u64 i=0;i<length;i++){
		if (ptr_a[i]!=ptr_b[i]){
			return ptr_a[i]-ptr_b[i];
		}
	}
	return 0;
}



SYS_PUBLIC void sys_memory_set(void* dst,u64 length,u8 value){
	u8* dst_ptr=dst;
	for (u64 i=0;i<length;i++){
		dst_ptr[i]=value;
	}
}
