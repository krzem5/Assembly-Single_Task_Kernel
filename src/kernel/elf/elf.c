#include <kernel/cpu/cpu.h>
#include <kernel/elf/elf.h>
#include <kernel/elf/structures.h>
#include <kernel/error/error.h>
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
#include <kernel/vfs/permissions.h>
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



typedef struct _ELF_LOADER_CONTEXT{
	const char* path;
	u32 argc;
	const char*const* argv;
	u32 environ_length;
	const char*const* environ;
	process_t* process;
	thread_t* thread;
	void* data;
	const elf_hdr_t* elf_header;
	u64 user_phdr_address;
	const char* interpreter_path;
	u64 interpreter_image_base;
} elf_loader_context_t;



static pmm_counter_descriptor_t* _user_image_pmm_counter=NULL;
static pmm_counter_descriptor_t* _user_input_data_pmm_counter=NULL;
static u32 _elf_hwcap=0;



static vfs_node_t* _get_executable_file(const char* path){
	vfs_node_t* out=vfs_lookup(NULL,path,VFS_LOOKUP_FLAG_CHECK_PERMISSIONS|VFS_LOOKUP_FLAG_FOLLOW_LINKS,THREAD_DATA->process->uid,THREAD_DATA->process->gid);
	if (!out){
		ERROR("Unable to find executable '%s'",path);
		return NULL;
	}
	if ((vfs_permissions_get(out,THREAD_DATA->process->uid,THREAD_DATA->process->gid)&(VFS_PERMISSION_READ|VFS_PERMISSION_EXEC))!=(VFS_PERMISSION_READ|VFS_PERMISSION_EXEC)){
		ERROR("File '%s' is not readable or executable",path);
		return NULL;
	}
	return out;
}



static error_t _check_elf_header(elf_loader_context_t* ctx){
	if (ctx->elf_header->e_ident.signature!=0x464c457f){
		ERROR("ELF header error: e_ident.signature != 0x464c457f");
		return ERROR_INVALID_FORMAT;
	}
	if (ctx->elf_header->e_ident.word_size!=2){
		ERROR("ELF header error: e_ident.word_size != 2");
		return ERROR_INVALID_FORMAT;
	}
	if (ctx->elf_header->e_ident.endianess!=1){
		ERROR("ELF header error: e_ident.endianess != 1");
		return ERROR_INVALID_FORMAT;
	}
	if (ctx->elf_header->e_ident.header_version!=1){
		ERROR("ELF header error: e_ident.header_version != 1");
		return ERROR_INVALID_FORMAT;
	}
	if (ctx->elf_header->e_ident.abi!=0){
		ERROR("ELF header error: e_ident.abi != 0");
		return ERROR_INVALID_FORMAT;
	}
	if (ctx->elf_header->e_type!=ET_EXEC){
		ERROR("ELF header error: e_type != ET_EXEC");
		return ERROR_INVALID_FORMAT;
	}
	if (ctx->elf_header->e_machine!=0x3e){
		ERROR("ELF header error: machine != 0x3e");
		return ERROR_INVALID_FORMAT;
	}
	if (ctx->elf_header->e_version!=1){
		ERROR("ELF header error: version != 1");
		return ERROR_INVALID_FORMAT;
	}
	return ERROR_OK;
}



static error_t _map_and_locate_sections(elf_loader_context_t* ctx){
	INFO("Mapping and locating sections...");
	for (u16 i=0;i<ctx->elf_header->e_phnum;i++){
		const elf_phdr_t* program_header=ctx->data+ctx->elf_header->e_phoff+i*ctx->elf_header->e_phentsize;
		if (program_header->p_type==PT_PHDR){
			ctx->user_phdr_address=program_header->p_vaddr;
			continue;
		}
		if (program_header->p_type==PT_INTERP){
			if (ctx->interpreter_path){
				ERROR("Multiple PT_INTERP program headers");
				return ERROR_INVALID_FORMAT;
			}
			ctx->interpreter_path=ctx->data+program_header->p_offset;
			if (ctx->interpreter_path[program_header->p_filesz-1]){
				ERROR("Interpreter string not null-terminated");
				return ERROR_INVALID_FORMAT;
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
		mmap_region_t* program_region=mmap_alloc(&(ctx->process->mmap),program_header->p_vaddr-padding,pmm_align_up_address(program_header->p_memsz+padding),_user_image_pmm_counter,flags,NULL,0);
		if (!program_region){
			return ERROR_NO_MEMORY;
		}
		mmap_set_memory(&(ctx->process->mmap),program_region,padding,ctx->data+program_header->p_offset,program_header->p_filesz);
	}
	return ERROR_OK;
}



static void _create_executable_thread(elf_loader_context_t* ctx){
	INFO("Creating main thread...");
	ctx->thread=thread_create_user_thread(ctx->process,ctx->elf_header->e_entry,0x200000);
}



static error_t _load_interpreter(elf_loader_context_t* ctx){
	if (!ctx->interpreter_path){
		return ERROR_OK;
	}
	INFO("Loading interpreter...");
	vfs_node_t* file=_get_executable_file(ctx->interpreter_path);
	if (!file){
		return ERROR_NOT_FOUND;
	}
	mmap_region_t* region=mmap_alloc(&(process_kernel->mmap),0,0,NULL,MMAP_REGION_FLAG_NO_FILE_WRITEBACK|MMAP_REGION_FLAG_VMM_NOEXECUTE|MMAP_REGION_FLAG_VMM_READWRITE,file,0);
	void* file_data=(void*)(region->rb_node.key);
	elf_hdr_t header=*((elf_hdr_t*)file_data);
	error_t out=ERROR_OK;
	if (header.e_ident.signature!=0x464c457f||header.e_ident.word_size!=2||header.e_ident.endianess!=1||header.e_ident.header_version!=1||header.e_ident.abi!=0||header.e_type!=ET_DYN||header.e_machine!=0x3e||header.e_version!=1){
		out=ERROR_INVALID_FORMAT;
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
	mmap_region_t* program_region=mmap_alloc(&(ctx->process->mmap),0,max_address,_user_image_pmm_counter,MMAP_REGION_FLAG_COMMIT|MMAP_REGION_FLAG_VMM_USER|MMAP_REGION_FLAG_VMM_READWRITE,NULL,0);
	if (!program_region){
		ERROR("Unable to allocate interpreter program memory");
		out=ERROR_NO_MEMORY;
		goto _error;
	}
	ctx->interpreter_image_base=program_region->rb_node.key;
	for (u16 i=0;i<header.e_phnum;i++){
		const elf_phdr_t* program_header=file_data+header.e_phoff+i*header.e_phentsize;
		if (program_header->p_type!=PT_LOAD){
			continue;
		}
		mmap_set_memory(&(ctx->process->mmap),program_region,program_header->p_vaddr,file_data+program_header->p_offset,program_header->p_filesz);
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
		elf_sym_t* symbol=symbol_table+(relocations->r_info>>32)*symbol_table_entry_size;
		switch (relocations->r_info&0xffffffff){
			case R_X86_64_GLOB_DAT:
				symbol->st_value+=ctx->interpreter_image_base;
				break;
			case R_X86_64_RELATIVE:
				symbol->st_value=ctx->interpreter_image_base+relocations->r_addend;
				break;
			default:
				ERROR("Unknown relocation type: %u",relocations->r_info);
				out=ERROR_INVALID_FORMAT;
				goto _error;
		}
		mmap_set_memory(&(ctx->process->mmap),program_region,relocations->r_offset,&(symbol->st_value),sizeof(u64));
		if (relocation_size<=relocation_entry_size){
			break;
		}
		relocations=(const elf_rela_t*)(((u64)relocations)+relocation_entry_size);
		relocation_size-=relocation_entry_size;
	}
_skip_dynamic_section:
	mmap_dealloc_region(&(process_kernel->mmap),region);
	ctx->thread->reg_state.gpr_state.rip=header.e_entry+ctx->interpreter_image_base;
	return ERROR_OK;
_error:
	mmap_dealloc_region(&(process_kernel->mmap),region);
	return out;
}



static error_t _generate_input_data(elf_loader_context_t* ctx){
	INFO("Generating input data...");
	u64 size=sizeof(u64);
	u64 string_table_size=0;
	for (u64 i=0;i<ctx->argc;i++){
		size+=sizeof(u64);
		string_table_size+=smm_length(ctx->argv[i])+1;
	}
	for (u64 i=0;i<ctx->environ_length;i++){
		size+=sizeof(u64);
		string_table_size+=smm_length(ctx->environ[i])+1;
	}
	size+=sizeof(u64); // environ NULL-terminator
	string_table_size+=smm_length(ELF_AUXV_PLATFORM)+1;
	string_table_size+=ELF_AUXV_RANDOM_DATA_SIZE+1;
	string_table_size+=smm_length(ctx->path)+1;
	size+=13*sizeof(elf_auxv_t); // auxiliary vector entries
	u64 total_size=size+((string_table_size+7)&0xfffffff8);
	mmap_region_t* region=mmap_alloc(&(ctx->process->mmap),0,pmm_align_up_address(total_size),_user_input_data_pmm_counter,MMAP_REGION_FLAG_COMMIT|MMAP_REGION_FLAG_VMM_NOEXECUTE|MMAP_REGION_FLAG_VMM_READWRITE|MMAP_REGION_FLAG_VMM_USER,NULL,0);
	if (!region){
		ERROR("Unable to reserve process input data memory");
		return ERROR_NO_MEMORY;
	}
	void* buffer=(void*)(pmm_alloc(pmm_align_up_address(total_size)>>PAGE_SIZE_SHIFT,_user_input_data_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	u64* data_ptr=buffer;
	void* string_table_ptr=buffer+size;
	PUSH_DATA_VALUE(ctx->argc);
	for (u64 i=0;i<ctx->argc;i++){
		PUSH_DATA_VALUE(string_table_ptr-buffer+region->rb_node.key);
		PUSH_STRING(ctx->argv[i]);
	}
	for (u64 i=0;i<ctx->environ_length;i++){
		PUSH_DATA_VALUE(string_table_ptr-buffer+region->rb_node.key);
		PUSH_STRING(ctx->environ[i]);
	}
	PUSH_DATA_VALUE(0); // environ NULL-terminator
	PUSH_AUXV_VALUE(AT_PHDR,ctx->user_phdr_address);
	PUSH_AUXV_VALUE(AT_PHENT,ctx->elf_header->e_phentsize);
	PUSH_AUXV_VALUE(AT_PHNUM,ctx->elf_header->e_phnum);
	PUSH_AUXV_VALUE(AT_PAGESZ,PAGE_SIZE);
	PUSH_AUXV_VALUE(AT_BASE,ctx->interpreter_image_base);
	PUSH_AUXV_VALUE(AT_FLAGS,0);
	PUSH_AUXV_VALUE(AT_ENTRY,ctx->elf_header->e_entry);
	PUSH_AUXV_VALUE(AT_PLATFORM,string_table_ptr-buffer+region->rb_node.key);
	PUSH_STRING(ELF_AUXV_PLATFORM);
	PUSH_AUXV_VALUE(AT_HWCAP,_elf_hwcap);
	PUSH_AUXV_VALUE(AT_RANDOM,string_table_ptr-buffer+region->rb_node.key);
	random_generate(string_table_ptr,ELF_AUXV_RANDOM_DATA_SIZE);
	string_table_ptr+=ELF_AUXV_RANDOM_DATA_SIZE;
	PUSH_AUXV_VALUE(AT_HWCAP2,0);
	PUSH_AUXV_VALUE(AT_EXECFN,string_table_ptr-buffer+region->rb_node.key);
	PUSH_STRING(ctx->path);
	PUSH_AUXV_VALUE(AT_NULL,0);
	mmap_set_memory(&(ctx->process->mmap),region,0,buffer,total_size);
	pmm_dealloc(((u64)buffer)-VMM_HIGHER_HALF_ADDRESS_OFFSET,pmm_align_up_address(total_size)>>PAGE_SIZE_SHIFT,_user_input_data_pmm_counter);
	ctx->thread->reg_state.gpr_state.r15=region->rb_node.key;
	return ERROR_OK;
}



KERNEL_INIT(){
	LOG("Initializing ELF loader...");
	_user_image_pmm_counter=pmm_alloc_counter("user_image");
	_user_input_data_pmm_counter=pmm_alloc_counter("user_input_data");
	_elf_hwcap=elf_get_hwcap();
}



KERNEL_PUBLIC error_t elf_load(const char* path,u32 argc,const char*const* argv,u32 environ_length,const char*const* environ,u32 flags){
	if (!path){
		return ERROR_INVALID_ARGUMENT(0);
	}
	if (!argv){
		argc=1;
		argv=&path;
	}
	LOG("Loading executable '%s'...",path);
	vfs_node_t* file=_get_executable_file(path);
	if (!file){
		return ERROR_NOT_FOUND;
	}
	process_t* process=process_create(path,file->name->data);
	mmap_region_t* region=mmap_alloc(&(process_kernel->mmap),0,0,NULL,MMAP_REGION_FLAG_NO_FILE_WRITEBACK|MMAP_REGION_FLAG_VMM_NOEXECUTE|MMAP_REGION_FLAG_VMM_READWRITE,file,0);
	INFO("Executable file size: %v",region->length);
	elf_loader_context_t ctx={
		path,
		argc,
		argv,
		environ_length,
		environ,
		process,
		NULL,
		(void*)(region->rb_node.key),
		(void*)(region->rb_node.key),
		0,
		NULL,
		0
	};
	error_t out=_check_elf_header(&ctx);
	if (out!=ERROR_OK){
		goto _error;
	}
	out=_map_and_locate_sections(&ctx);
	if (out!=ERROR_OK){
		goto _error;
	}
	_create_executable_thread(&ctx);
	out=_load_interpreter(&ctx);
	if (out!=ERROR_OK){
		goto _error;
	}
	out=_generate_input_data(&ctx);
	if (out!=ERROR_OK){
		goto _error;
	}
	mmap_dealloc_region(&(process_kernel->mmap),region);
	if (!(flags&ELF_LOAD_FLAG_PAUSE_THREAD)){
		scheduler_enqueue_thread(ctx.thread);
	}
	return process->handle.rb_node.key;
_error:
	mmap_dealloc_region(&(process_kernel->mmap),region);
	handle_release(&(process->handle));
	return out;
}
