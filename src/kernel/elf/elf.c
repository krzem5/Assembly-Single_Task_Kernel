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



_Bool elf_load(vfs_node_t* node){
	if (!node){
		return 0;
	}
	process_t* process=process_new();
	mmap_region_t* region=mmap_alloc(&(process_kernel->mmap),0,0,NULL,MMAP_REGION_FLAG_NO_FILE_WRITEBACK|MMAP_REGION_FLAG_VMM_NOEXECUTE|MMAP_REGION_FLAG_VMM_READWRITE,node);
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
		u64 flags=VMM_PAGE_FLAG_USER|VMM_PAGE_FLAG_PRESENT;
		if (!(program_header->p_flags&PF_X)){
			flags|=VMM_PAGE_FLAG_NOEXECUTE;
		}
		if (program_header->p_flags&PF_W){
			flags|=VMM_PAGE_FLAG_READWRITE;
		}
		u64 offset=program_header->p_vaddr&(PAGE_SIZE-1);
		u64 page_count=pmm_align_up_address(program_header->p_memsz+offset)>>PAGE_SIZE_SHIFT;
		u64 pages=pmm_alloc_zero(page_count,&_user_image_pmm_counter,0);
		if (!mmap_alloc(&(process->mmap),program_header->p_vaddr-offset,page_count<<PAGE_SIZE_SHIFT,&_user_image_pmm_counter,0,NULL)){
			goto _error;
		}
		vmm_map_pages(&(process->pagemap),pages,program_header->p_vaddr-offset,flags,page_count);
		memcpy((void*)(pages+offset+VMM_HIGHER_HALF_ADDRESS_OFFSET),file_data+program_header->p_offset,program_header->p_filesz);
	}
	scheduler_enqueue_thread(thread_new_user_thread(process,header.e_entry,0x200000));
	return 1;
_error:
	mmap_dealloc(&(process_kernel->mmap),region->rb_node.key,region->length);
	handle_release(&(process->handle));
	return 0;
}
