#include <kernel/aslr/aslr.h>
#include <kernel/cpu/cpu.h>
#include <kernel/elf/elf.h>
#include <kernel/elf/structures.h>
#include <kernel/error/error.h>
#include <kernel/fd/fd.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/random/random.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/signature/signature.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/permissions.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "elf"



#define ELF_ASLR_MMAP_BOTTOM_OFFSET_MIN 0x8000000 // 128 MB
#define ELF_ASLR_MMAP_BOTTOM_OFFSET_MAX 0x40000000 // 1 GB
#define ELF_ASLR_MMAP_TOP_MIN 0x7fff80000000 // -2 GB
#define ELF_ASLR_MMAP_TOP_MAX 0x7fffc0000000 // -1 GB
#define ELF_ASLR_STACK_TOP_MIN 0x7fffe0000000ull // -512 MB
#define ELF_ASLR_STACK_TOP_MAX 0x7ffffffff000ull // -4 KB

#define ELF_STACK_SIZE 0x800000 // 8 MB

#define ELF_STACK_ALIGNMENT 16

#define ELF_DYN_LOAD_ADDRESS 0x400000

#define PUSH_DATA_VALUE(value) *(data_ptr++)=(u64)(value)
#define PUSH_AUXV_VALUE(type,value) PUSH_DATA_VALUE((type));PUSH_DATA_VALUE((value))
#define PUSH_STRING(string) \
	do{ \
		u32 __length=smm_length((string))+1; \
		mem_copy(string_table_ptr,(string),__length); \
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
	u64 entry_address;
	u64 stack_top;
} elf_loader_context_t;



static pmm_counter_descriptor_t* _user_image_pmm_counter=NULL;
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
	if (ctx->elf_header->e_ident.signature!=0x464c457f||ctx->elf_header->e_ident.word_size!=2||ctx->elf_header->e_ident.endianess!=1||ctx->elf_header->e_ident.header_version!=1||ctx->elf_header->e_ident.abi!=0||(ctx->elf_header->e_type!=ET_EXEC&&ctx->elf_header->e_type!=ET_DYN)||ctx->elf_header->e_machine!=0x3e||ctx->elf_header->e_version!=1){
		return ERROR_INVALID_FORMAT;
	}
	return ERROR_OK;
}



static void _create_executable_process(elf_loader_context_t* ctx,const char* image,const char* name){
	INFO("Creating process...");
	u64 highest_address=0;
	for (u16 i=0;i<ctx->elf_header->e_phnum;i++){
		const elf_phdr_t* program_header=ctx->data+ctx->elf_header->e_phoff+i*ctx->elf_header->e_phentsize;
		if (program_header->p_type!=PT_LOAD){
			continue;
		}
		u64 end=program_header->p_vaddr+program_header->p_memsz;
		if (end>highest_address){
			highest_address=end;
		}
	}
	ctx->process=process_create(image,name,pmm_align_up_address(highest_address)+aslr_generate_address(ELF_ASLR_MMAP_BOTTOM_OFFSET_MIN,ELF_ASLR_MMAP_BOTTOM_OFFSET_MAX),aslr_generate_address(ELF_ASLR_MMAP_TOP_MIN,ELF_ASLR_MMAP_TOP_MAX));
	ctx->stack_top=aslr_generate_address(ELF_ASLR_STACK_TOP_MIN,ELF_ASLR_STACK_TOP_MAX);
	if (!mmap_alloc(ctx->process->mmap,ctx->stack_top-ELF_STACK_SIZE,ELF_STACK_SIZE,MMAP_REGION_FLAG_STACK|MMAP_REGION_FLAG_VMM_WRITE|MMAP_REGION_FLAG_VMM_USER|MMAP_REGION_FLAG_FORCE,NULL)){
		panic("Unable to allocate stack");
	}
}



static error_t _map_and_locate_sections(elf_loader_context_t* ctx){
	INFO("Mapping and locating sections...");
	const elf_dyn_t* dyn=NULL;
	for (u16 i=0;i<ctx->elf_header->e_phnum;i++){
		elf_phdr_t* program_header=ctx->data+ctx->elf_header->e_phoff+i*ctx->elf_header->e_phentsize;
		if (ctx->elf_header->e_type==ET_DYN){
			program_header->p_vaddr+=ELF_DYN_LOAD_ADDRESS;
		}
		if (program_header->p_type==PT_DYNAMIC){
			dyn=ctx->data+program_header->p_offset;
			continue;
		}
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
				ERROR("Interpreter string is not null-terminated");
				return ERROR_INVALID_FORMAT;
			}
			continue;
		}
		if (program_header->p_type!=PT_LOAD){
			continue;
		}
		u64 padding=program_header->p_vaddr&(PAGE_SIZE-1);
		u32 flags=MMAP_REGION_FLAG_VMM_USER|MMAP_REGION_FLAG_FORCE;
		if (program_header->p_flags&PF_X){
			flags|=MMAP_REGION_FLAG_VMM_EXEC;
		}
		if (program_header->p_flags&PF_W){
			flags|=MMAP_REGION_FLAG_VMM_WRITE;
		}
		mmap_region_t* region=mmap_alloc(ctx->process->mmap,program_header->p_vaddr-padding,pmm_align_up_address(program_header->p_memsz+padding),flags,NULL);
		if (!region){
			return ERROR_NO_MEMORY;
		}
		if (!program_header->p_filesz){
			continue;
		}
		mmap_region_t* kernel_region=mmap_map_to_kernel(ctx->process->mmap,program_header->p_vaddr-padding,pmm_align_up_address(program_header->p_filesz+padding));
		mem_copy((void*)(kernel_region->rb_node.key+padding),ctx->data+program_header->p_offset,program_header->p_filesz);
		mmap_dealloc_region(process_kernel->mmap,kernel_region);
	}
	if (ctx->elf_header->e_type!=ET_DYN||!dyn){
		return ERROR_OK;
	}
	void* symbol_table=NULL;
	u64 symbol_table_entry_size=0;
	const elf_rela_t* relocations=NULL;
	u64 relocation_size=0;
	u64 relocation_entry_size=0;
	for (;dyn->d_tag!=DT_NULL;dyn++){
		switch (dyn->d_tag){
			case DT_SYMTAB:
				symbol_table=ctx->data+((u64)(dyn->d_un.d_ptr));
				break;
			case DT_SYMENT:
				symbol_table_entry_size=dyn->d_un.d_val;
				break;
			case DT_RELA:
				relocations=ctx->data+((u64)(dyn->d_un.d_ptr));
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
		return ERROR_OK;
	}
	(void)symbol_table;
	(void)symbol_table_entry_size;
	(void)relocation_size;
	(void)relocation_entry_size;
	ERROR("_map_and_locate_sections: apply relocations");
	return ERROR_INVALID_SYSCALL;
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
	mmap_region_t* region=mmap_alloc(process_kernel->mmap,0,0,MMAP_REGION_FLAG_NO_WRITEBACK|MMAP_REGION_FLAG_VMM_WRITE,file);
	mmap_region_t* kernel_program_region=NULL;
	void* file_data=(void*)(region->rb_node.key);
	elf_hdr_t header=*((elf_hdr_t*)file_data);
	error_t out=ERROR_OK;
	if (header.e_ident.signature!=0x464c457f||header.e_ident.word_size!=2||header.e_ident.endianess!=1||header.e_ident.header_version!=1||header.e_ident.abi!=0||header.e_type!=ET_DYN||header.e_machine!=0x3e||header.e_version!=1){
		out=ERROR_INVALID_FORMAT;
		goto _error;
	}
	if (!signature_verify_user(ctx->interpreter_path,region)){
		out=ERROR_DENIED;
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
	mmap_region_t* program_region=mmap_alloc(ctx->process->mmap,0,max_address,MMAP_REGION_FLAG_COMMIT|MMAP_REGION_FLAG_VMM_USER,NULL);
	if (!program_region){
		ERROR("Unable to allocate interpreter program memory");
		out=ERROR_NO_MEMORY;
		goto _error;
	}
	kernel_program_region=mmap_map_to_kernel(ctx->process->mmap,program_region->rb_node.key,max_address);
	ctx->interpreter_image_base=program_region->rb_node.key;
	for (u16 i=0;i<header.e_phnum;i++){
		const elf_phdr_t* program_header=file_data+header.e_phoff+i*header.e_phentsize;
		if (program_header->p_type!=PT_LOAD){
			continue;
		}
		mem_copy((void*)(kernel_program_region->rb_node.key+program_header->p_vaddr),file_data+program_header->p_offset,program_header->p_filesz);
		u64 flags=0;
		if (!(program_header->p_flags&PF_X)){
			flags|=VMM_PAGE_FLAG_NOEXECUTE;
		}
		if (program_header->p_flags&PF_W){
			flags|=VMM_PAGE_FLAG_READWRITE;
		}
		u64 padding=program_header->p_vaddr&(PAGE_SIZE-1);
		vmm_adjust_flags(&(ctx->process->pagemap),program_region->rb_node.key+program_header->p_vaddr-padding,flags,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE,pmm_align_up_address(program_header->p_memsz+padding),0);
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
			case R_X86_64_64:
				symbol->st_value+=relocations->r_addend;
				break;
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
		*((u64*)(kernel_program_region->rb_node.key+relocations->r_offset))=symbol->st_value;
		if (relocation_size<=relocation_entry_size){
			break;
		}
		relocations=(const elf_rela_t*)(((u64)relocations)+relocation_entry_size);
		relocation_size-=relocation_entry_size;
	}
_skip_dynamic_section:
	if (kernel_program_region){
		mmap_dealloc_region(process_kernel->mmap,kernel_program_region);
	}
	mmap_dealloc_region(process_kernel->mmap,region);
	ctx->entry_address=ctx->interpreter_image_base+header.e_entry;
	return ERROR_OK;
_error:
	if (kernel_program_region){
		mmap_dealloc_region(process_kernel->mmap,kernel_program_region);
	}
	mmap_dealloc_region(process_kernel->mmap,region);
	return out;
}



static void _create_executable_thread(elf_loader_context_t* ctx){
	INFO("Creating main thread...");
	ctx->thread=thread_create_user_thread(ctx->process,ctx->entry_address,ctx->stack_top);
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
	u64 total_size=(size+string_table_size+ELF_STACK_ALIGNMENT-1)&(-ELF_STACK_ALIGNMENT);
	if (total_size>ELF_STACK_SIZE){
		ERROR("Stack too small for arguments");
		return ERROR_NO_SPACE;
	}
	mmap_region_t* kernel_region=mmap_map_to_kernel(ctx->process->mmap,ctx->stack_top-pmm_align_up_address(total_size),pmm_align_up_address(total_size));
	ctx->stack_top-=total_size;
	void* buffer=(void*)(kernel_region->rb_node.key+((-total_size)&(PAGE_SIZE-1)));
	u64* data_ptr=buffer;
	void* string_table_ptr=buffer+size;
	u64 pointer_difference=((void*)(ctx->stack_top))-buffer;
	PUSH_DATA_VALUE(ctx->argc);
	for (u64 i=0;i<ctx->argc;i++){
		PUSH_DATA_VALUE(string_table_ptr+pointer_difference);
		PUSH_STRING(ctx->argv[i]);
	}
	for (u64 i=0;i<ctx->environ_length;i++){
		PUSH_DATA_VALUE(string_table_ptr+pointer_difference);
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
	PUSH_AUXV_VALUE(AT_PLATFORM,string_table_ptr+pointer_difference);
	PUSH_STRING(ELF_AUXV_PLATFORM);
	PUSH_AUXV_VALUE(AT_HWCAP,_elf_hwcap);
	PUSH_AUXV_VALUE(AT_RANDOM,string_table_ptr+pointer_difference);
	random_generate(string_table_ptr,ELF_AUXV_RANDOM_DATA_SIZE);
	string_table_ptr+=ELF_AUXV_RANDOM_DATA_SIZE;
	PUSH_AUXV_VALUE(AT_HWCAP2,0);
	PUSH_AUXV_VALUE(AT_EXECFN,string_table_ptr+pointer_difference);
	PUSH_STRING(ctx->path);
	PUSH_AUXV_VALUE(AT_NULL,0);
	mmap_dealloc_region(process_kernel->mmap,kernel_region);
	return ERROR_OK;
}



KERNEL_INIT(){
	LOG("Initializing ELF loader...");
	_user_image_pmm_counter=pmm_alloc_counter("user_image");
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
	mmap_region_t* region=mmap_alloc(process_kernel->mmap,0,0,MMAP_REGION_FLAG_NO_WRITEBACK|MMAP_REGION_FLAG_VMM_WRITE,file);
	INFO("Executable file size: %v",region->length);
	elf_loader_context_t ctx={
		path,
		argc,
		argv,
		environ_length,
		environ,
		NULL,
		NULL,
		(void*)(region->rb_node.key),
		(void*)(region->rb_node.key),
		0,
		NULL,
		0,
		0,
		0
	};
	ctx.entry_address=ctx.elf_header->e_entry;
	error_t out=_check_elf_header(&ctx);
	if (out!=ERROR_OK){
		goto _error;
	}
	if (!signature_verify_user(path,region)){
		out=ERROR_DENIED;
		goto _error;
	}
	_create_executable_process(&ctx,path,file->name->data);
	out=_map_and_locate_sections(&ctx);
	if (out!=ERROR_OK){
		goto _error;
	}
	out=_load_interpreter(&ctx);
	if (out!=ERROR_OK){
		goto _error;
	}
	out=_generate_input_data(&ctx);
	if (out!=ERROR_OK){
		goto _error;
	}
	_create_executable_thread(&ctx);
	mmap_dealloc_region(process_kernel->mmap,region);
	if (!(flags&ELF_LOAD_FLAG_PAUSE_THREAD)){
		scheduler_enqueue_thread(ctx.thread);
	}
	return ctx.process->handle.rb_node.key;
_error:
	if (ctx.thread){
		ctx.thread->state=THREAD_STATE_TYPE_TERMINATED;
		thread_delete(ctx.thread);
	}
	if (ctx.process){
		handle_release(&(ctx.process->handle));
	}
	mmap_dealloc_region(process_kernel->mmap,region);
	return out;
}
