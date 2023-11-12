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
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "elf"



static pmm_counter_descriptor_t _user_image_pmm_counter=PMM_COUNTER_INIT_STRUCT("user_image");



static void _load_interpreter(process_t* process,const char* path,thread_t* thread){
	vfs_node_t* file=vfs_lookup(NULL,path,1);
	if (!file){
		panic("Unable to find interpreter");
	}
	mmap_region_t* region=mmap_alloc(&(process_kernel->mmap),0,0,NULL,MMAP_REGION_FLAG_NO_FILE_WRITEBACK|MMAP_REGION_FLAG_VMM_NOEXECUTE|MMAP_REGION_FLAG_VMM_READWRITE,file);
	void* file_data=(void*)(region->rb_node.key);
	elf_hdr_t header=*((elf_hdr_t*)file_data);
	if (header.e_ident.signature!=0x464c457f||header.e_ident.word_size!=2||header.e_ident.endianess!=1||header.e_ident.header_version!=1||header.e_ident.abi!=0||header.e_type!=ET_DYN||header.e_machine!=0x3e||header.e_version!=1){
		goto _error;
	}
	u64 max_address=0;
	const elf_dyn_t* dyn=NULL;
	for (u16 i=0;i<header.e_phnum;i++){
		const elf_phdr_t* program_header=file_data+header.e_phoff+i*header.e_phentsize;
		if (program_header->p_type==PT_DYNAMIC){
			dyn=file_data+program_header->p_offset;
			continue;
		}
		if (program_header->p_type!=PT_LOAD){
			continue;
		}
		u64 address=pmm_align_up_address(program_header->p_vaddr+program_header->p_memsz);
		if (address>max_address){
			max_address=address;
		}
	}
	mmap_region_t* program_region=mmap_alloc(&(process->mmap),0,max_address,&_user_image_pmm_counter,MMAP_REGION_FLAG_COMMIT|MMAP_REGION_FLAG_VMM_READWRITE|MMAP_REGION_FLAG_VMM_USER,NULL);
	if (!program_region){
		goto _error;
	}
	u64 image_base=program_region->rb_node.key;
	for (u16 i=0;i<header.e_phnum;i++){
		const elf_phdr_t* program_header=file_data+header.e_phoff+i*header.e_phentsize;
		if (program_header->p_type!=PT_LOAD){
			continue;
		}
		mmap_set_memory(&(process->mmap),program_region,program_header->p_vaddr,file_data+program_header->p_offset,program_header->p_filesz);
	}
	if (!dyn){
		goto _skip_dynamic_section;
	}
	void* symbol_table=NULL;
	u64 symbol_table_entry_size=0;
	const elf_rela_t* relocations=NULL;
	u64 relocation_size=0;
	u64 relocation_entry_size=0;
	for (;dyn->d_tag!=DT_NULL;dyn++){
		switch (dyn->d_tag){
			case DT_SYMTAB:
				symbol_table=file_data+dyn->d_un.d_ptr;
				break;
			case DT_SYMENT:
				symbol_table_entry_size=dyn->d_un.d_val;
				break;
			case DT_RELA:
				relocations=file_data+dyn->d_un.d_ptr;
				break;
			case DT_RELASZ:
				relocation_size=dyn->d_un.d_val;
				break;
			case DT_RELAENT:
				relocation_entry_size=dyn->d_un.d_val;
				break;
		}
	}
	if (!relocations){
		goto _skip_dynamic_section;
	}
	while (1){
		switch (relocations->r_info&0xffffffff){
			case R_X86_64_GLOB_DAT:
				elf_sym_t* symbol=symbol_table+(relocations->r_info>>32)*symbol_table_entry_size;
				symbol->st_value+=image_base;
				mmap_set_memory(&(process->mmap),program_region,relocations->r_offset,&(symbol->st_value),sizeof(u64));
				break;
			default:
				ERROR("Unknown relocation type: %u",relocations->r_info);
				panic("Unknown relocation type");
				break;
		}
		if (relocation_size<=relocation_entry_size){
			break;
		}
		relocations=(const elf_rela_t*)(((u64)relocations)+relocation_entry_size);
		relocation_size-=relocation_entry_size;
	}
_skip_dynamic_section:
	mmap_dealloc_region(&(process_kernel->mmap),region);
	// thread->gpr_state.rip=header.e_entry+image_base;
	return;
_error:
	mmap_dealloc_region(&(process_kernel->mmap),region);
}



_Bool elf_load(vfs_node_t* file){
	if (!file){
		return 0;
	}
	char image_buffer[4096];
	vfs_path(file,image_buffer,4096);
	process_t* process=process_new(image_buffer,file->name->data);
	mmap_region_t* region=mmap_alloc(&(process_kernel->mmap),0,0,NULL,MMAP_REGION_FLAG_NO_FILE_WRITEBACK|MMAP_REGION_FLAG_VMM_NOEXECUTE|MMAP_REGION_FLAG_VMM_READWRITE,file);
	void* file_data=(void*)(region->rb_node.key);
	elf_hdr_t header=*((elf_hdr_t*)file_data);
	if (header.e_ident.signature!=0x464c457f||header.e_ident.word_size!=2||header.e_ident.endianess!=1||header.e_ident.header_version!=1||header.e_ident.abi!=0||header.e_type!=ET_EXEC||header.e_machine!=0x3e||header.e_version!=1){
		goto _error;
	}
	const char* interpreter=NULL;
	for (u16 i=0;i<header.e_phnum;i++){
		const elf_phdr_t* program_header=file_data+header.e_phoff+i*header.e_phentsize;
		if (program_header->p_type==PT_INTERP){
			if (interpreter){
				ERROR("Multiple PT_INTERP program headers");
				return 0;
			}
			interpreter=file_data+program_header->p_offset;
			if (interpreter[program_header->p_filesz-1]){
				ERROR("Interpreter string not null-terminated");
				return 0;
			}
			continue;
		}
		if (program_header->p_type!=PT_LOAD){
			continue;
		}
		u64 padding=program_header->p_vaddr&(PAGE_SIZE-1);
		u64 flags=MMAP_REGION_FLAG_COMMIT|MMAP_REGION_FLAG_VMM_USER;
		if (!(program_header->p_flags&PF_X)){
			flags|=MMAP_REGION_FLAG_VMM_NOEXECUTE;
		}
		if (program_header->p_flags&PF_W){
			flags|=MMAP_REGION_FLAG_VMM_READWRITE;
		}
		mmap_region_t* program_region=mmap_alloc(&(process->mmap),program_header->p_vaddr-padding,pmm_align_up_address(program_header->p_memsz+padding),&_user_image_pmm_counter,flags,NULL);
		if (!program_region){
			goto _error;
		}
		mmap_set_memory(&(process->mmap),program_region,padding,file_data+program_header->p_offset,program_header->p_filesz);
	}
	mmap_dealloc_region(&(process_kernel->mmap),region);
	thread_t* thread=thread_new_user_thread(process,header.e_entry,0x200000);
	if (interpreter){
		_load_interpreter(process,interpreter,thread);
	}
	scheduler_enqueue_thread(thread);
	return 1;
_error:
	mmap_dealloc_region(&(process_kernel->mmap),region);
	handle_release(&(process->handle));
	return 0;
}
