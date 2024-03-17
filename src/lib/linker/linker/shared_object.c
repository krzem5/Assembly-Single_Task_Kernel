#include <linker/alloc.h>
#include <linker/search_path.h>
#include <linker/shared_object.h>
#include <linker/symbol.h>
#include <sys/fd/fd.h>
#include <sys/io/io.h>
#include <sys/memory/memory.h>
#include <sys/string/string.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>
#ifdef KERNEL_COVERAGE
#include <sys/syscall/syscall.h>
#endif



static shared_object_t* _shared_object_tail=NULL;

shared_object_t* shared_object_root=NULL;



shared_object_t* shared_object_init(u64 image_base,const elf_dyn_t* dynamic_section,const char* path,u32 flags){
	u16 path_length=sys_string_length(path);
	if (path_length>=sizeof(((shared_object_t*)NULL)->path)){
		sys_io_print("Shared object path too long\n");
		return NULL;
	}
	shared_object_t* so=alloc(sizeof(shared_object_t));
	so->prev=_shared_object_tail;
	so->next=NULL;
	if (_shared_object_tail){
		_shared_object_tail->next=so;
	}
	else{
		shared_object_root=so;
	}
	_shared_object_tail=so;
	so->image_base=image_base;
	sys_memory_copy(path,so->path,path_length);
	so->path[path_length]=0;
#ifdef KERNEL_COVERAGE
	so->gcov_info_base=0;
	so->gcov_info_size=0;
#endif
	if (!dynamic_section){
		return so;
	}
	so->dynamic_section.has_needed_libraries=0;
	so->dynamic_section.plt_relocation_size=0;
	so->dynamic_section.plt_got=NULL;
	so->dynamic_section.hash_table=NULL;
	so->dynamic_section.string_table=NULL;
	so->dynamic_section.symbol_table=NULL;
	so->dynamic_section.relocations=NULL;
	so->dynamic_section.relocation_size=0;
	so->dynamic_section.relocation_entry_size=0;
	so->dynamic_section.symbol_table_entry_size=0;
	so->dynamic_section.plt_relocation_entry_size=0;
	so->dynamic_section.plt_relocations=NULL;
	so->dynamic_section.init_array=NULL;
	so->dynamic_section.init_array_size=0;
	so->dynamic_section.fini_array=NULL;
	so->dynamic_section.fini_array_size=0;
	for (const elf_dyn_t* dyn=dynamic_section;dyn->d_tag!=DT_NULL;dyn++){
		switch (dyn->d_tag){
			case DT_NEEDED:
				so->dynamic_section.has_needed_libraries=1;
				break;
			case DT_PLTRELSZ:
				so->dynamic_section.plt_relocation_size=dyn->d_un.d_val;
				break;
			case DT_PLTGOT:
				so->dynamic_section.plt_got=so->image_base+dyn->d_un.d_ptr;
				break;
			case DT_HASH:
				so->dynamic_section.hash_table=so->image_base+dyn->d_un.d_ptr;
				break;
			case DT_STRTAB:
				so->dynamic_section.string_table=so->image_base+dyn->d_un.d_ptr;
				break;
			case DT_SYMTAB:
				so->dynamic_section.symbol_table=so->image_base+dyn->d_un.d_ptr;
				break;
			case DT_RELA:
				so->dynamic_section.relocations=so->image_base+dyn->d_un.d_ptr;
				break;
			case DT_RELASZ:
				so->dynamic_section.relocation_size=dyn->d_un.d_val;
				break;
			case DT_RELAENT:
				so->dynamic_section.relocation_entry_size=dyn->d_un.d_val;
				break;
			case DT_SYMENT:
				so->dynamic_section.symbol_table_entry_size=dyn->d_un.d_val;
				break;
			case DT_INIT:
				so->dynamic_section.init=image_base+dyn->d_un.d_ptr;
				break;
			case DT_FINI:
				so->dynamic_section.fini=image_base+dyn->d_un.d_ptr;
				break;
			case DT_PLTREL:
				so->dynamic_section.plt_relocation_entry_size=(dyn->d_un.d_val==DT_RELA?sizeof(elf_rela_t):sizeof(elf_rel_t));
				break;
			case DT_JMPREL:
				so->dynamic_section.plt_relocations=so->image_base+dyn->d_un.d_ptr;
				break;
			case DT_BIND_NOW:
				flags|=SHARED_OBJECT_FLAG_RESOLVE_GOT;
				break;
			case DT_INIT_ARRAY:
				so->dynamic_section.init_array=so->image_base+dyn->d_un.d_ptr;
				break;
			case DT_FINI_ARRAY:
				so->dynamic_section.fini_array=so->image_base+dyn->d_un.d_ptr;
				break;
			case DT_INIT_ARRAYSZ:
				so->dynamic_section.init_array_size=dyn->d_un.d_val;
				break;
			case DT_FINI_ARRAYSZ:
				so->dynamic_section.fini_array_size=dyn->d_un.d_val;
				break;
		}
	}
	if (!so->dynamic_section.string_table||!so->dynamic_section.hash_table||!so->dynamic_section.symbol_table||!so->dynamic_section.symbol_table_entry_size){
		so->dynamic_section.hash_table=NULL;
	}
	if (so->dynamic_section.plt_got){
		so->dynamic_section.plt_got[1]=(u64)so;
		so->dynamic_section.plt_got[2]=(u64)symbol_resolve_plt_trampoline;
	}
	if (so->dynamic_section.has_needed_libraries&&so->dynamic_section.string_table){
		for (const elf_dyn_t* dyn=dynamic_section;dyn->d_tag!=DT_NULL;dyn++){
			if (dyn->d_tag==DT_NEEDED){
				shared_object_load(so->dynamic_section.string_table+dyn->d_un.d_val,flags);
			}
		}
	}
	if (so->dynamic_section.plt_got&&so->dynamic_section.plt_relocations&&so->dynamic_section.plt_relocation_size&&so->dynamic_section.plt_relocation_entry_size){
		for (u64 i=0;i<so->dynamic_section.plt_relocation_size/so->dynamic_section.plt_relocation_entry_size;i++){
			if (flags&SHARED_OBJECT_FLAG_RESOLVE_GOT){
				symbol_resolve_plt(so,i);
			}
			else{
				const elf_rela_t* relocation=so->dynamic_section.plt_relocations+i*so->dynamic_section.plt_relocation_entry_size;
				if ((relocation->r_info&0xffffffff)==R_X86_64_JUMP_SLOT){
					*(u64*)(so->image_base+relocation->r_offset)+=so->image_base;
				}
			}
		}
	}
	if (so->dynamic_section.relocations&&so->dynamic_section.relocation_size&&so->dynamic_section.relocation_entry_size&&so->dynamic_section.symbol_table&&so->dynamic_section.symbol_table_entry_size&&so->dynamic_section.string_table){
		for (u64 i=0;i<so->dynamic_section.relocation_size;i+=so->dynamic_section.relocation_entry_size){
			const elf_rela_t* relocation=so->dynamic_section.relocations+i;
			const elf_sym_t* symbol=so->dynamic_section.symbol_table+(relocation->r_info>>32)*so->dynamic_section.symbol_table_entry_size;
			switch (relocation->r_info&0xffffffff){
				case R_X86_64_COPY:
					sys_memory_copy((void*)symbol_lookup_by_name(so->dynamic_section.string_table+symbol->st_name),(void*)(so->image_base+relocation->r_offset),symbol->st_size);
					break;
				case R_X86_64_64:
				case R_X86_64_GLOB_DAT:
					*((u64*)(so->image_base+relocation->r_offset))=symbol_lookup_by_name(so->dynamic_section.string_table+symbol->st_name);
					break;
				case R_X86_64_RELATIVE:
					*((u64*)(so->image_base+relocation->r_offset))=so->image_base+relocation->r_addend;
					break;
				default:
					return NULL;
			}
		}
	}
	if (so->dynamic_section.init){
		((void (*)(void))(so->dynamic_section.init))();
	}
	if (so->dynamic_section.init_array&&so->dynamic_section.init_array_size){
		for (u64 i=0;i+sizeof(void*)<=so->dynamic_section.init_array_size;i+=sizeof(void*)){
			void* func=*((void*const*)(so->dynamic_section.init_array+i));
			if (func&&func!=(void*)0xffffffffffffffffull){
				((void (*)(void))func)();
			}
		}
	}
	return so;
}



shared_object_t* shared_object_load(const char* name,u32 flags){
	char buffer[256];
	sys_fd_t fd=search_path_find_library(name,buffer,256);
	if (SYS_IS_ERROR(fd)){
		sys_io_print("Unable to find library '%s'\n",name);
		return NULL;
	}
	for (shared_object_t* so=shared_object_root;so;so=so->next){
		if (!sys_string_compare(so->path,buffer)){
			return so;
		}
	}
	void* base_file_address=(void*)sys_memory_map(0,SYS_MEMORY_FLAG_READ|SYS_MEMORY_FLAG_WRITE|SYS_MEMORY_FLAG_FILE|SYS_MEMORY_FLAG_NOWRITEBACK,fd);
	sys_fd_close(fd);
	_sys_syscall_signature_verify(buffer,base_file_address,sys_memory_get_size(base_file_address));
	const elf_hdr_t* header=base_file_address;
	if (header->e_ident.signature!=0x464c457f||header->e_ident.word_size!=2||header->e_ident.endianess!=1||header->e_ident.header_version!=1||header->e_ident.abi!=0||header->e_type!=ET_DYN||header->e_machine!=0x3e||header->e_version!=1){
		sys_memory_unmap((void*)base_file_address,0);
		return NULL;
	}
	u64 max_address=0;
	for (u16 i=0;i<header->e_phnum;i++){
		const elf_phdr_t* program_header=(void*)(base_file_address+header->e_phoff+i*header->e_phentsize);
		if (program_header->p_type!=PT_LOAD){
			continue;
		}
		u64 address=program_header->p_vaddr+program_header->p_memsz;
		if (address>max_address){
			max_address=address;
		}
	}
	void* image_base=(void*)sys_memory_map(sys_memory_align_up_address(max_address),SYS_MEMORY_FLAG_WRITE,0);
#ifdef KERNEL_COVERAGE
	u64 gcov_info_base=0;
	u64 gcov_info_size=0;
	const char* section_header_name_string_table=base_file_address+((const elf_shdr_t*)(base_file_address+header->e_shoff+header->e_shstrndx*header->e_shentsize))->sh_offset;
	for (u16 i=0;i<header->e_shnum;i++){
		const elf_shdr_t* section_header=(void*)(base_file_address+header->e_shoff+i*header->e_shentsize);
		if (!sys_string_compare(section_header_name_string_table+section_header->sh_name,".gcov_info")){
			gcov_info_base=((u64)image_base)+section_header->sh_addr;
			gcov_info_size=section_header->sh_size;
		}
	}
#endif
	const elf_dyn_t* dynamic_section=NULL;
	for (u16 i=0;i<header->e_phnum;i++){
		const elf_phdr_t* program_header=(void*)(base_file_address+header->e_phoff+i*header->e_phentsize);
		if (program_header->p_type==PT_DYNAMIC){
			dynamic_section=image_base+program_header->p_vaddr;
			continue;
		}
		if (program_header->p_type!=PT_LOAD){
			continue;
		}
		u64 flags=0;
		if (program_header->p_flags&PF_R){
			flags|=SYS_MEMORY_FLAG_READ;
		}
		if (program_header->p_flags&PF_W){
			flags|=SYS_MEMORY_FLAG_WRITE;
		}
		if (program_header->p_flags&PF_X){
			flags|=SYS_MEMORY_FLAG_EXEC;
		}
		sys_memory_copy(base_file_address+program_header->p_offset,image_base+program_header->p_vaddr,program_header->p_filesz);
		sys_memory_change_flags(image_base+program_header->p_vaddr,program_header->p_memsz,flags);
	}
	shared_object_t* so=shared_object_init((u64)image_base,dynamic_section,buffer,flags);
#ifdef KERNEL_COVERAGE
	so->gcov_info_base=gcov_info_base;
	so->gcov_info_size=gcov_info_size;
#endif
	sys_memory_unmap((void*)base_file_address,0);
	return so;
}



void shared_object_execute_fini(void){
	for (shared_object_t* so=_shared_object_tail;so;so=so->prev){
		if (so->dynamic_section.fini){
			((void (*)(void))(so->dynamic_section.fini))();
		}
		if (so->dynamic_section.fini_array&&so->dynamic_section.fini_array_size){
			for (u64 i=0;i+sizeof(void*)<=so->dynamic_section.fini_array_size;i+=sizeof(void*)){
				void* func=*((void*const*)(so->dynamic_section.fini_array+i));
				if (func&&func!=(void*)0xffffffffffffffffull){
					((void (*)(void))func)();
				}
			}
		}
	}
}



#ifdef KERNEL_COVERAGE
SYS_PUBLIC void SYS_DESTRUCTOR SYS_NOCOVERAGE __sys_linker_dump_coverage(void){
	u64 syscall_table_offset=sys_syscall_get_table_offset("coverage");
	for (shared_object_t* so=_shared_object_tail;so;so=so->prev){
		if (so->gcov_info_base&&so->gcov_info_size){
			_sys_syscall2(syscall_table_offset|0x00000001,so->gcov_info_base,so->gcov_info_size);
		}
	}
}
#endif
