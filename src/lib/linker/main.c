#include <core/elf.h>
#include <core/fd.h>
#include <core/io.h>
#include <core/memory.h>
#include <core/types.h>
#include <linker/resolver.h>



typedef struct _DYNAMIC_SECTION_DATA{
	_Bool has_needed_libraries;
	u64 plt_relocation_size;
	u64* plt_got;
	const elf_hash_t* hash_table;
	const char* string_table;
	const void* symbol_table;
	const void* relocations;
	u64 relocation_size;
	u64 relocation_entry_size;
	u64 symbol_table_entry_size;
	u64 plt_relocation_entry_size;
	const void* plt_relocations;
} dynamic_section_data_t;



typedef struct _SHARED_OBJECT{
	struct _SHARED_OBJECT* next;
	u64 image_base;
	const void* elf_phdr_entries;
	u64 elf_phdr_entry_size;
	u64 elf_phdr_entry_count;
	const elf_dyn_t* elf_dynamic_section;
	dynamic_section_data_t dynamic_section;
} shared_object_t;



static shared_object_t* root_shared_object;
static shared_object_t* tail_shared_object;



static shared_object_t* _alloc_shared_object(void){
	static shared_object_t buffer[16];
	static u8 index=0;
	return buffer+(index++);
}



static _Bool _init_shared_object(shared_object_t* so);



static void _parse_dynamic_section(const elf_dyn_t* dynamic_section,u64 image_base,dynamic_section_data_t* out){
	out->has_needed_libraries=0;
	out->plt_relocation_size=0;
	out->plt_got=NULL;
	out->hash_table=NULL;
	out->string_table=NULL;
	out->symbol_table=NULL;
	out->relocations=NULL;
	out->relocation_size=0;
	out->relocation_entry_size=0;
	out->symbol_table_entry_size=0;
	out->plt_relocation_entry_size=0;
	out->plt_relocations=NULL;
	for (const elf_dyn_t* dyn=dynamic_section;dyn->d_tag!=DT_NULL;dyn++){
		switch (dyn->d_tag){
			case DT_NEEDED:
				out->has_needed_libraries=1;
				break;
			case DT_PLTRELSZ:
				out->plt_relocation_size=dyn->d_un.d_val;
				break;
			case DT_PLTGOT:
				out->plt_got=image_base+dyn->d_un.d_ptr;
				break;
			case DT_HASH:
				out->hash_table=image_base+dyn->d_un.d_ptr;
				break;
			case DT_STRTAB:
				out->string_table=image_base+dyn->d_un.d_ptr;
				break;
			case DT_SYMTAB:
				out->symbol_table=image_base+dyn->d_un.d_ptr;
				break;
			case DT_RELA:
				out->relocations=image_base+dyn->d_un.d_ptr;
				break;
			case DT_RELASZ:
				out->relocation_size=dyn->d_un.d_val;
				break;
			case DT_RELAENT:
				out->relocation_entry_size=dyn->d_un.d_val;
				break;
			case DT_SYMENT:
				out->symbol_table_entry_size=dyn->d_un.d_val;
				break;
			case DT_PLTREL:
				out->plt_relocation_entry_size=(dyn->d_un.d_val==DT_RELA?sizeof(elf_rela_t):sizeof(elf_rel_t));
				break;
			case DT_JMPREL:
				out->plt_relocations=image_base+dyn->d_un.d_ptr;
				break;
		}
	}
}



static u64 _load_root_shared_object_data(const u64* data,shared_object_t* so){
	so->next=NULL;
	so->elf_phdr_entries=NULL;
	so->elf_phdr_entry_size=0;
	so->elf_phdr_entry_count=0;
	u64 out=0;
	for (data+=(data[0]+1);data[0];data++);
	for (data++;data[0];data+=2){
		if (data[0]==AT_PHDR){
			so->elf_phdr_entries=(void*)data[1];
		}
		else if (data[0]==AT_PHENT){
			so->elf_phdr_entry_size=data[1];
		}
		else if (data[0]==AT_PHNUM){
			so->elf_phdr_entry_count=data[1];
		}
		else if (data[0]==AT_ENTRY){
			out=data[1];
		}
	}
	if (!so->elf_phdr_entries||!so->elf_phdr_entry_size||!so->elf_phdr_entry_count){
		printf("No PHDR supplied to the dynamic linker\n");
	}
	if (!out){
		printf("No entry address supplied to the dynamic linker\n");
	}
	so->elf_dynamic_section=NULL;
	for (u16 i=0;i<so->elf_phdr_entry_count;i++){
		const elf_phdr_t* program_header=so->elf_phdr_entries+i*so->elf_phdr_entry_size;
		if (program_header->p_type!=PT_DYNAMIC){
			continue;
		}
		so->elf_dynamic_section=(void*)(program_header->p_vaddr);
	}
	return out;
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
	void* base_file_address=memory_map(0,MEMORY_FLAG_NOWRITEBACK|MEMORY_FLAG_FILE|MEMORY_FLAG_WRITE|MEMORY_FLAG_READ,fd);
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
	void* image_base=memory_map(max_address,MEMORY_FLAG_EXEC|MEMORY_FLAG_WRITE|MEMORY_FLAG_READ,0);
	const elf_dyn_t* dynamic_section=NULL;
	for (u16 i=0;i<header->e_phnum;i++){
		elf_phdr_t* program_header=(void*)(base_file_address+header->e_phoff+i*header->e_phentsize);
		if (program_header->p_type==PT_DYNAMIC){
			dynamic_section=image_base+program_header->p_vaddr;
			continue;
		}
		if (program_header->p_type!=PT_LOAD){
			continue;
		}
		program_header->p_vaddr+=(u64)image_base;
		memcpy((void*)(program_header->p_vaddr),base_file_address+program_header->p_offset,program_header->p_filesz);
	}
	memory_unmap(base_file_address,0);
	shared_object_t* so=_alloc_shared_object();
	root_shared_object->next=so;
	tail_shared_object=so;
	so->next=NULL;
	so->image_base=(u64)image_base;
	so->elf_phdr_entries=image_base+header->e_phoff;
	so->elf_phdr_entry_size=header->e_phentsize;
	so->elf_phdr_entry_count=header->e_phnum;
	so->elf_dynamic_section=dynamic_section;
	printf("Found library: %s -> %p [%v]\n",buffer,image_base,max_address);
	return _init_shared_object(so);
}



static u64 _lookup_symbol(const char* name){
	u32 hash=0;
	for (const char* tmp=name;tmp[0];tmp++){
		hash=(hash<<4)+tmp[0];
		hash^=(hash>>24)&0xf0;
	}
	hash&=0x0fffffff;
	for (const shared_object_t* so=root_shared_object;so;so=so->next){
		if (!so->dynamic_section.hash_table){
			continue;
		}
		for (u32 i=so->dynamic_section.hash_table->data[hash%so->dynamic_section.hash_table->nbucket];1;i=so->dynamic_section.hash_table->data[i+so->dynamic_section.hash_table->nbucket]){
			const elf_sym_t* symbol=so->dynamic_section.symbol_table+i*so->dynamic_section.symbol_table_entry_size;
			if (symbol->st_shndx==SHN_UNDEF){
				break;
			}
			const char* symbol_name=so->dynamic_section.string_table+symbol->st_name;
			for (u32 j=0;1;j++){
				if (name[j]!=symbol_name[j]){
					goto _skip_entry;
				}
				if (!name[j]){
					return so->image_base+symbol->st_value;
				}
			}
_skip_entry:
		}
	}
	return 0;
}



extern void _resolve_symbol_trampoline(void);



u64 _resolve_symbol(shared_object_t* so,u64 index){
	const elf_rela_t* relocation=so->dynamic_section.plt_relocations+index*so->dynamic_section.plt_relocation_entry_size;
	if ((relocation->r_info&0xffffffff)!=R_X86_64_JUMP_SLOT){
		return 0;
	}
	const elf_sym_t* symbol=so->dynamic_section.symbol_table+(relocation->r_info>>32)*so->dynamic_section.symbol_table_entry_size;
	u64 resolved_symbol=_lookup_symbol(so->dynamic_section.string_table+symbol->st_name);
	if (!resolved_symbol){
		return 0;
	}
	if (so->dynamic_section.plt_relocation_entry_size==sizeof(elf_rela_t)){
		resolved_symbol+=relocation->r_addend;
	}
	*((u64*)(so->image_base+relocation->r_offset))=resolved_symbol;
	return resolved_symbol;
}



static _Bool _init_shared_object(shared_object_t* so){
	if (!so->elf_dynamic_section){
		return 1;
	}
	_parse_dynamic_section(so->elf_dynamic_section,so->image_base,&(so->dynamic_section));
	if (so->dynamic_section.plt_got){
		so->dynamic_section.plt_got[1]=(u64)so;
		so->dynamic_section.plt_got[2]=(u64)_resolve_symbol_trampoline;
	}
	if (!so->dynamic_section.string_table||!so->dynamic_section.hash_table||!so->dynamic_section.symbol_table||!so->dynamic_section.symbol_table_entry_size){
		so->dynamic_section.hash_table=NULL;
	}
	if (so->dynamic_section.has_needed_libraries&&so->dynamic_section.string_table){
		for (const elf_dyn_t* dyn=so->elf_dynamic_section;dyn->d_tag!=DT_NULL;dyn++){
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
					memcpy((void*)(so->image_base+relocation->r_offset),(void*)_lookup_symbol(so->dynamic_section.string_table+symbol->st_name),symbol->st_size);
					break;
				case R_X86_64_GLOB_DAT:
					*((u64*)(so->image_base+relocation->r_offset))=_lookup_symbol(so->dynamic_section.string_table+symbol->st_name);
					break;
				case R_X86_64_RELATIVE:
					*((u64*)(so->image_base+relocation->r_offset))=so->image_base+relocation->r_addend;
					break;
				default:
					printf("Unknown relocation type: %u\n",relocation->r_info);
					return 0;
			}
		}
	}
	return 1;
}



u64 main(const u64* data){
	const u64* base_data=data;
	const char* string_table=(const char*)data;
	u32 argc=data[0];
	data++;
	printf("argc=%u\n",argc);
	for (u32 i=0;i<argc;i++){
		printf("argv[%u]=\"%s\"\n",i,string_table+data[0]);
		data++;
	}
	for (u32 i=0;data[0];i++){
		printf("environ[%u]=\"%s\"\n",i,string_table+data[0]);
		data++;
	}
	for (data++;data[0];data+=2){
		if (data[0]==AT_IGNORE){
			continue;
		}
		switch (data[0]){
			case AT_PHDR:
				printf("AT_PHDR=%p\n",data[1]);
				break;
			case AT_PHENT:
				printf("AT_PHENT=%v\n",data[1]);
				break;
			case AT_PHNUM:
				printf("AT_PHNUM=%lu\n",data[1]);
				break;
			case AT_PAGESZ:
				printf("AT_PAGESZ=%lu B\n",data[1]);
				break;
			case AT_BASE:
				printf("AT_BASE=%p\n",data[1]);
				break;
			case AT_FLAGS:
				printf("AT_FLAGS=%u\n",data[1]);
				break;
			case AT_ENTRY:
				printf("AT_ENTRY=%p\n",data[1]);
				break;
			case AT_PLATFORM:
				printf("AT_PLATFORM=\"%s\"\n",string_table+data[1]);
				break;
			case AT_HWCAP:
				printf("AT_HWCAP=%x\n",data[1]);
				break;
			case AT_RANDOM:
				printf("AT_RANDOM=%p\n",string_table+data[1]);
				break;
			case AT_HWCAP2:
				printf("AT_HWCAP2=%x\n",data[1]);
				break;
			case AT_EXECFN:
				printf("AT_EXECFN=\"%s\"\n",string_table+data[1]);
				break;
			default:
				printf("AT_%u=%lx\n",data[0],data[1]);
				break;
		}
	}
	root_shared_object=_alloc_shared_object();
	u64 entry_address=_load_root_shared_object_data(base_data,root_shared_object);
	_init_shared_object(root_shared_object);
	return entry_address;
}
