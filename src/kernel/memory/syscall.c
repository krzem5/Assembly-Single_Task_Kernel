#include <kernel/fd/fd.h>
#include <kernel/isr/isr.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/thread.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>



#define MEMORY_FLAG_READ 1
#define MEMORY_FLAG_WRITE 2
#define MEMORY_FLAG_EXEC 4
#define MEMORY_FLAG_FILE 8
#define MEMORY_FLAG_NOWRITEBACK 16



static pmm_counter_descriptor_t _user_data_pmm_counter=PMM_COUNTER_INIT_STRUCT("user_data");



void syscall_memory_map(isr_state_t* regs){
	u64 flags=MMAP_REGION_FLAG_VMM_USER;
	vfs_node_t* file=NULL;
	if (regs->rsi&MEMORY_FLAG_WRITE){
		flags|=MMAP_REGION_FLAG_VMM_READWRITE;
	}
	if (!(regs->rsi&MEMORY_FLAG_EXEC)){
		flags|=MMAP_REGION_FLAG_VMM_NOEXECUTE;
	}
	if (regs->rsi&MEMORY_FLAG_FILE){
		file=fd_get_node(regs->rdx);
		if (!file){
			regs->rax=0;
			return;
		}
	}
	if (regs->rsi&MEMORY_FLAG_NOWRITEBACK){
		flags|=MMAP_REGION_FLAG_NO_FILE_WRITEBACK;
	}
	u64 length=pmm_align_up_address(regs->rdi);
	mmap_region_t* out=mmap_alloc(&(THREAD_DATA->process->mmap),0,length,&_user_data_pmm_counter,flags,file);
	regs->rax=(out?out->rb_node.key:0);
}



void syscall_memory_unmap(isr_state_t* regs){
	regs->rax=mmap_dealloc(&(THREAD_DATA->process->mmap),regs->rdi,pmm_align_up_address(regs->rsi));
}
