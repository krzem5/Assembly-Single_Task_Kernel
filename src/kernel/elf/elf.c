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
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "elf"



static pmm_counter_descriptor_t _user_image_pmm_counter=PMM_COUNTER_INIT_STRUCT("user_image");



_Bool elf_load(vfs_node_t* node){
	if (!node){
		return 0;
	}
	process_t* process=process_new();
	elf_hdr_t header;
	if (vfs_node_read(node,0,&header,sizeof(elf_hdr_t))!=sizeof(elf_hdr_t)||header.e_ident.signature!=0x464c457f||header.e_ident.word_size!=2||header.e_ident.endianess!=1||header.e_ident.header_version!=1||header.e_ident.abi!=0||header.e_type!=ET_EXEC||header.e_machine!=0x3e||header.e_version!=1){
		goto _error;
	}
	u64 highest_address=0;
	for (u16 i=0;i<header.e_phnum;i++){
		elf_phdr_t program_header;
		if (vfs_node_read(node,header.e_phoff+i*sizeof(elf_phdr_t),&program_header,sizeof(elf_phdr_t))!=sizeof(elf_phdr_t)){
			goto _error;
		}
		if (program_header.p_type!=PT_LOAD){
			continue;
		}
		u64 flags=VMM_PAGE_FLAG_USER|VMM_PAGE_FLAG_PRESENT;
		if (!(program_header.p_flags&PF_X)){
			flags|=VMM_PAGE_FLAG_NOEXECUTE;
		}
		if (program_header.p_flags&PF_W){
			flags|=VMM_PAGE_FLAG_READWRITE;
		}
		u64 offset=program_header.p_vaddr&(PAGE_SIZE-1);
		u64 page_count=pmm_align_up_address(program_header.p_memsz+offset)>>PAGE_SIZE_SHIFT;
		u64 pages=pmm_alloc_zero(page_count,&_user_image_pmm_counter,0);
		if (!mmap_reserve(&(process->mmap),program_header.p_vaddr-offset,page_count<<PAGE_SIZE_SHIFT,&_user_image_pmm_counter)){
			goto _error;
		}
		vmm_map_pages(&(process->pagemap),pages,program_header.p_vaddr-offset,flags,page_count);
		u64 end_address=program_header.p_vaddr-offset+(page_count<<PAGE_SIZE_SHIFT);
		if (end_address>highest_address){
			highest_address=end_address;
		}
		if (vfs_node_read(node,program_header.p_offset,(void*)(pages+offset+VMM_HIGHER_HALF_ADDRESS_OFFSET),program_header.p_filesz)!=program_header.p_filesz){
			goto _error;
		}
	}
	scheduler_enqueue_thread(thread_new_user_thread(process,header.e_entry,0x200000));
	return 1;
_error:
	handle_release(&(process->handle));
	return 0;
}
