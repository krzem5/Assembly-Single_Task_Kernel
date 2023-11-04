#include <kernel/cpu/cpu.h>
#include <kernel/elf/structures.h>
#include <kernel/fd/fd.h>
#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "elf"



static pmm_counter_descriptor_t _user_image_pmm_counter=PMM_COUNTER_INIT_STRUCT("user_image");



_Bool elf_load(vfs_node_t* file){
	if (!file){
		return 0;
	}
	process_t* process=process_new();
	mmap_region_t* region=mmap_alloc(&(process_kernel->mmap),0,0,NULL,MMAP_REGION_FLAG_NO_FILE_WRITEBACK|MMAP_REGION_FLAG_VMM_NOEXECUTE|MMAP_REGION_FLAG_VMM_READWRITE,file);
	void* file_data=(void*)(region->rb_node.key);
	elf_hdr_t header=*((elf_hdr_t*)file_data);
	if (header.e_ident.signature!=0x464c457f||header.e_ident.word_size!=2||header.e_ident.endianess!=1||header.e_ident.header_version!=1||header.e_ident.abi!=0||header.e_type!=ET_EXEC||header.e_machine!=0x3e||header.e_version!=1){
		goto _error;
	}
	for (u16 i=0;i<header.e_phnum;i++){
		const elf_phdr_t* program_header=file_data+header.e_phoff+i*sizeof(elf_phdr_t);
		if (program_header->p_type!=PT_LOAD){
			continue;
		}
		if (program_header->p_vaddr&(PAGE_SIZE-1)){
			panic("elf_load: Non-page-aligned program header");
		}
		u64 flags=MMAP_REGION_FILE_OFFSET(program_header->p_offset)|MMAP_REGION_FLAG_NO_FILE_WRITEBACK|MMAP_REGION_FLAG_COMMIT|MMAP_REGION_FLAG_VMM_USER;
		if (!(program_header->p_flags&PF_X)){
			flags|=MMAP_REGION_FLAG_VMM_NOEXECUTE;
		}
		if (program_header->p_flags&PF_W){
			flags|=MMAP_REGION_FLAG_VMM_READWRITE;
		}
		if (!mmap_alloc(&(process->mmap),program_header->p_vaddr,pmm_align_up_address(program_header->p_memsz),&_user_image_pmm_counter,flags,file)){
			goto _error;
		}
	}
	mmap_dealloc_region(&(process_kernel->mmap),region);
	scheduler_enqueue_thread(thread_new_user_thread(process,header.e_entry,0x200000));
	return 1;
_error:
	mmap_dealloc_region(&(process_kernel->mmap),region);
	handle_release(&(process->handle));
	return 0;
}
