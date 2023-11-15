#include <core/elf.h>
#include <core/fd.h>
#include <core/io.h>
#include <core/memory.h>
#include <core/types.h>
#include <linker/resolver.h>
#include <linker/shared_object.h>
#include <linker/symbol.h>



static _Bool _init_shared_object(shared_object_t* so,const elf_dyn_t* dynamic_section);



static const elf_dyn_t* _load_root_shared_object_data(const u64* data,u64* out){
	const void* phdr_entries=NULL;
	u64 phdr_entry_size=0;
	u64 phdr_entry_count=0;
	*out=0;
	for (data+=(data[0]+1);data[0];data++);
	for (data++;data[0];data+=2){
		if (data[0]==AT_PHDR){
			phdr_entries=(void*)(data[1]);
		}
		else if (data[0]==AT_PHENT){
			phdr_entry_size=data[1];
		}
		else if (data[0]==AT_PHNUM){
			phdr_entry_count=data[1];
		}
		else if (data[0]==AT_ENTRY){
			*out=data[1];
		}
	}
	if (!phdr_entries||!phdr_entry_size||!phdr_entry_count){
		printf("No PHDR supplied to the dynamic linker\n");
	}
	if (!(*out)){
		printf("No entry address supplied to the dynamic linker\n");
	}
	for (u16 i=0;i<phdr_entry_count;i++){
		const elf_phdr_t* program_header=phdr_entries+i*phdr_entry_size;
		if (program_header->p_type==PT_DYNAMIC){
			return (void*)(program_header->p_vaddr);
		}
	}
	return NULL;
}



static _Bool _check_elf_header(const elf_hdr_t* header){
	if (header->e_ident.signature!=0x464c457f){
		printf("ELF header error: e_ident.signature != 0x464c457f\n");
		return 0;
	}
	if (header->e_ident.word_size!=2){
		printf("ELF header error: e_ident.word_size != 2\n");
		return 0;
	}
	if (header->e_ident.endianess!=1){
		printf("ELF header error: e_ident.endianess != 1\n");
		return 0;
	}
	if (header->e_ident.header_version!=1){
		printf("ELF header error: e_ident.header_version != 1\n");
		return 0;
	}
	if (header->e_ident.abi!=0){
		printf("ELF header error: e_ident.abi != 0\n");
		return 0;
	}
	if (header->e_type!=ET_DYN){
		printf("ELF header error: e_type != ET_EXEC\n");
		return 0;
	}
	if (header->e_machine!=0x3e){
		printf("ELF header error: machine != 0x3e\n");
		return 0;
	}
	if (header->e_version!=1){
		printf("ELF header error: version != 1\n");
		return 0;
	}
	return 1;
}



static void* memcpy(void* dst,const void* src,u64 length){
	u8* dst_ptr=dst;
	const u8* src_ptr=src;
	for (u64 i=0;i<length;i++){
		dst_ptr[i]=src_ptr[i];
	}
	return dst;
}



static _Bool _load_shared_object(const char* name){
	char buffer[4096];
	s64 fd=resolve_library_name(name,buffer,4096);
	if (!fd){
		printf("Unable to find library '%s'\n",name);
		return 0;
	}
	const void* base_file_address=memory_map(0,MEMORY_FLAG_NOWRITEBACK|MEMORY_FLAG_FILE|MEMORY_FLAG_READ,fd);
	fd_close(fd);
	const elf_hdr_t* header=base_file_address;
	if (!_check_elf_header(header)){
		return 0;
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
	max_address=(max_address+4095)&0xfffffffffffff000ull;
	void* image_base=memory_map(max_address,MEMORY_FLAG_WRITE,0);
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
			flags|=MEMORY_FLAG_READ;
		}
		if (program_header->p_flags&PF_W){
			flags|=MEMORY_FLAG_WRITE;
		}
		if (program_header->p_flags&PF_X){
			flags|=MEMORY_FLAG_EXEC;
		}
		memcpy(image_base+program_header->p_vaddr,base_file_address+program_header->p_offset,program_header->p_filesz);
		memory_change_flags(image_base+program_header->p_vaddr,program_header->p_memsz,flags);
	}
	memory_unmap((void*)base_file_address,0);
	return _init_shared_object(shared_object_alloc((u64)image_base),dynamic_section);
}



static _Bool _init_shared_object(shared_object_t* so,const elf_dyn_t* dynamic_section){
	if (!dynamic_section){
		return 1;
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
			case DT_PLTREL:
				so->dynamic_section.plt_relocation_entry_size=(dyn->d_un.d_val==DT_RELA?sizeof(elf_rela_t):sizeof(elf_rel_t));
				break;
			case DT_JMPREL:
				so->dynamic_section.plt_relocations=so->image_base+dyn->d_un.d_ptr;
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
				_load_shared_object(so->dynamic_section.string_table+dyn->d_un.d_val);
			}
		}
	}
	if (so->dynamic_section.relocations&&so->dynamic_section.relocation_size&&so->dynamic_section.relocation_entry_size){
		for (u64 i=0;i<so->dynamic_section.relocation_size;i+=so->dynamic_section.relocation_entry_size){
			const elf_rela_t* relocation=so->dynamic_section.relocations+i;
			const elf_sym_t* symbol=so->dynamic_section.symbol_table+(relocation->r_info>>32)*so->dynamic_section.symbol_table_entry_size;
			switch (relocation->r_info&0xffffffff){
				case R_X86_64_COPY:
					memcpy((void*)(so->image_base+relocation->r_offset),(void*)symbol_lookup_by_name(so->dynamic_section.string_table+symbol->st_name),symbol->st_size);
					break;
				case R_X86_64_GLOB_DAT:
					*((u64*)(so->image_base+relocation->r_offset))=symbol_lookup_by_name(so->dynamic_section.string_table+symbol->st_name);
					break;
				case R_X86_64_RELATIVE:
					*((u64*)(so->image_base+relocation->r_offset))=so->image_base+relocation->r_addend;
					break;
				default:
					printf("Unknown relocation type: %u\n",relocation->r_info&0xffffffff);
					return 0;
			}
		}
	}
	return 1;
}



u64 main(const u64* data){
	u64 entry_address;
	_init_shared_object(shared_object_alloc(0),_load_root_shared_object_data(data,&entry_address));
	return entry_address;
}
