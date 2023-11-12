#include <kernel/cpu/cpu.h>
#include <kernel/elf/elf.h>
#include <kernel/elf/structures.h>
#include <kernel/fd/fd.h>
#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/random/random.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "elf"



#define PUSH_DATA_VALUE(value) *(data_ptr++)=(u64)(value)
#define PUSH_AUXV_VALUE(type,value) PUSH_DATA_VALUE((type));PUSH_DATA_VALUE((value))
#define PUSH_STRING(string) \
	do{ \
		u32 __length=smm_length((string))+1; \
		memcpy(string_table_ptr,(string),__length); \
		string_table_ptr+=__length; \
	} while (0)



static pmm_counter_descriptor_t _user_image_pmm_counter=PMM_COUNTER_INIT_STRUCT("user_image");
static pmm_counter_descriptor_t _user_input_data_pmm_counter=PMM_COUNTER_INIT_STRUCT("user_input_data");



static void _init_input_data(process_t* process,thread_t* thread,const elf_hdr_t* elf_header){
	u64 interpreter_image_base=0x11223344aabbccddull;
	u32 argc=3;
	const char* argv[]={process->image->data,"arg1","arg2"};
	const char* environ[]={"aaa","bbb","ccc","ddd",NULL};
	u64 size=sizeof(u64);
	u64 string_table_size=0;
	for (u64 i=0;i<argc;i++){
		size+=sizeof(u64);
		string_table_size+=smm_length(argv[i])+1;
	}
	for (u64 i=0;environ[i];i++){
		size+=sizeof(u64);
		string_table_size+=smm_length(environ[i])+1;
	}
	size+=sizeof(u64); // environ NULL-terminator
	string_table_size+=smm_length(ELF_AUXV_PLATFORM)+1;
	string_table_size+=ELF_AUXV_RANDOM_DATA_SIZE+1;
	string_table_size+=smm_length(process->image->data)+1;
	size+=13*sizeof(elf_auxv_t); // auxiliary vector entries
	u64 total_size=size+((string_table_size+7)&0xfffffff8);
	void* buffer=(void*)(pmm_alloc(pmm_align_up_address(total_size)>>PAGE_SIZE_SHIFT,&_user_input_data_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	u64* data_ptr=buffer;
	void* string_table_ptr=buffer+size;
	PUSH_DATA_VALUE(argc);
	for (u64 i=0;i<argc;i++){
		PUSH_DATA_VALUE(string_table_ptr-buffer);
		PUSH_STRING(argv[i]);
	}
	for (u64 i=0;environ[i];i++){
		PUSH_DATA_VALUE(string_table_ptr-buffer);
		PUSH_STRING(environ[i]);
	}
	PUSH_DATA_VALUE(0); // environ NULL-terminator
	PUSH_AUXV_VALUE(AT_PHDR,elf_header->e_phoff);
	PUSH_AUXV_VALUE(AT_PHENT,elf_header->e_phentsize);
	PUSH_AUXV_VALUE(AT_PHNUM,elf_header->e_phnum);
	PUSH_AUXV_VALUE(AT_PAGESZ,PAGE_SIZE);
	PUSH_AUXV_VALUE(AT_BASE,interpreter_image_base);
	PUSH_AUXV_VALUE(AT_FLAGS,0);
	PUSH_AUXV_VALUE(AT_ENTRY,elf_header->e_entry);
	PUSH_AUXV_VALUE(AT_PLATFORM,string_table_ptr-buffer);
	PUSH_STRING(ELF_AUXV_PLATFORM);
	PUSH_AUXV_VALUE(AT_HWCAP,0); // cpuid[1].edx
	PUSH_AUXV_VALUE(AT_RANDOM,string_table_ptr-buffer);
	random_generate(string_table_ptr,ELF_AUXV_RANDOM_DATA_SIZE);
	string_table_ptr+=ELF_AUXV_RANDOM_DATA_SIZE;
	PUSH_AUXV_VALUE(AT_HWCAP2,0);
	PUSH_AUXV_VALUE(AT_EXECFN,string_table_ptr-buffer);
	PUSH_STRING(process->image->data);
	PUSH_AUXV_VALUE(AT_NULL,0);
	mmap_region_t* region=mmap_alloc(&(process->mmap),0,pmm_align_up_address(total_size),&_user_input_data_pmm_counter,MMAP_REGION_FLAG_COMMIT|MMAP_REGION_FLAG_VMM_NOEXECUTE|MMAP_REGION_FLAG_VMM_USER,NULL);
	if (!region){
		panic("Unable to reserve process input data memory");
	}
	mmap_set_memory(&(process->mmap),region,0,buffer,total_size);
	pmm_dealloc(((u64)buffer)-VMM_HIGHER_HALF_ADDRESS_OFFSET,pmm_align_up_address(total_size)>>PAGE_SIZE_SHIFT,&_user_input_data_pmm_counter);
	thread->gpr_state.r15=region->rb_node.key;
}



static u64 _load_interpreter(process_t* process,const char* path){
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
				symbol_table=file_data+((u64)(dyn->d_un.d_ptr));
				break;
			case DT_SYMENT:
				symbol_table_entry_size=dyn->d_un.d_val;
				break;
			case DT_RELA:
				relocations=file_data+((u64)(dyn->d_un.d_ptr));
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
	return header.e_entry+image_base;
_error:
	mmap_dealloc_region(&(process_kernel->mmap),region);
	return 0;
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
	_init_input_data(process,thread,&header);
	if (interpreter){
		/*thread->gpr_state.rip=*/_load_interpreter(process,interpreter);
	}
	scheduler_enqueue_thread(thread);
	return 1;
_error:
	mmap_dealloc_region(&(process_kernel->mmap),region);
	handle_release(&(process->handle));
	return 0;
}
