#include <core/elf.h>
#include <core/fd.h>
#include <core/io.h>
#include <core/memory.h>
#include <core/types.h>
#include <linker/resolver.h>
#include <linker/symbol_table.h>



typedef struct _LINKER_CONTEXT{
	const void* elf_phdr_entries;
	u64 elf_phdr_entry_size;
	u64 elf_phdr_entry_count;
	u64 elf_entry_address;
	const elf_dyn_t* elf_dynamic_section;
	const char* elf_string_table;
	const elf_hash_t* elf_hash_table;
	const void* elf_symbol_table;
	u64 elf_symbol_table_entry_size;
} linker_context_t;



static void _load_linker_context(const u64* data,linker_context_t* ctx){
	ctx->elf_phdr_entries=NULL;
	ctx->elf_phdr_entry_size=0;
	ctx->elf_phdr_entry_count=0;
	ctx->elf_entry_address=0;
	for (data+=(data[0]+1);data[0];data++);
	for (data++;data[0];data+=2){
		if (data[0]==AT_PHDR){
			ctx->elf_phdr_entries=(void*)data[1];
		}
		else if (data[0]==AT_PHENT){
			ctx->elf_phdr_entry_size=data[1];
		}
		else if (data[0]==AT_PHNUM){
			ctx->elf_phdr_entry_count=data[1];
		}
		else if (data[0]==AT_ENTRY){
			ctx->elf_entry_address=data[1];
		}
	}
	if (!ctx->elf_phdr_entries||!ctx->elf_phdr_entry_size||!ctx->elf_phdr_entry_count){
		printf("No PHDR supplied to the dynamic linker\n");
	}
	if (!ctx->elf_entry_address){
		printf("No entry address supplied to the dynamic linker\n");
	}
}



static void _find_dynamic_section(linker_context_t* ctx){
	ctx->elf_dynamic_section=NULL;
	for (u16 i=0;i<ctx->elf_phdr_entry_count;i++){
		const elf_phdr_t* program_header=ctx->elf_phdr_entries+i*ctx->elf_phdr_entry_size;
		if (program_header->p_type!=PT_DYNAMIC){
			continue;
		}
		ctx->elf_dynamic_section=(void*)(program_header->p_vaddr);
	}
}



static void _load_symbols(linker_context_t* ctx,u64 image_base){
	for (u32 i=0;i<ctx->elf_hash_table->nbucket;i++){
		for (u32 j=ctx->elf_hash_table->data[i];1;j=ctx->elf_hash_table->data[ctx->elf_hash_table->nbucket+j]){
			const elf_sym_t* symbol=ctx->elf_symbol_table+j*ctx->elf_symbol_table_entry_size;
			if (symbol->st_shndx==SHN_UNDEF){
				break;
			}
			symbol_table_add(ctx->elf_string_table+symbol->st_name,image_base+symbol->st_value);
		}
	}
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
	void* image_base=memory_map(max_address,MEMORY_FLAG_WRITE|MEMORY_FLAG_READ,0);
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
	if (!dynamic_section){
		return 1;
	}
	const void* hash_table=NULL;
	const char* string_table=NULL;
	void* symbol_table=NULL;
	u64 symbol_table_entry_size=0;
	const elf_rela_t* relocations=NULL;
	u64 relocation_size=0;
	u64 relocation_entry_size=0;
	for (const elf_dyn_t* dyn=dynamic_section;dyn->d_tag!=DT_NULL;dyn++){
		switch (dyn->d_tag){
			case DT_HASH:
				hash_table=image_base+((u64)(dyn->d_un.d_ptr));
				break;
			case DT_STRTAB:
				string_table=image_base+((u64)(dyn->d_un.d_ptr));
				break;
			case DT_SYMTAB:
				symbol_table=image_base+((u64)(dyn->d_un.d_ptr));
				break;
			case DT_SYMENT:
				symbol_table_entry_size=dyn->d_un.d_val;
				break;
			case DT_RELA:
				relocations=image_base+((u64)(dyn->d_un.d_ptr));
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
				*((u64*)(image_base+relocations->r_offset))=symbol->st_value+((u64)image_base);
				break;
			case R_X86_64_RELATIVE:
				*((u64*)(image_base+relocations->r_offset))=((u64)image_base)+relocations->r_addend;
				break;
			default:
				printf("Unknown relocation type: %u\n",relocations->r_info);
				return 0;
		}
		if (relocation_size<=relocation_entry_size){
			break;
		}
		relocations=(const elf_rela_t*)(((u64)relocations)+relocation_entry_size);
		relocation_size-=relocation_entry_size;
	}
_skip_dynamic_section:
	linker_context_t ctx={
		image_base+header->e_phoff,
		header->e_phentsize,
		header->e_phnum,
		0,
		dynamic_section,
		string_table,
		hash_table,
		symbol_table,
		symbol_table_entry_size
	};
	printf("Found library: %s -> %p [%p]\n",buffer,image_base,max_address);
	_load_symbols(&ctx,(u64)image_base);
	return 1;
}



extern void _resolve_symbol_trampoline(void);



static u64 __TEST_FUNCTION_STUB(void){
	return 12345;
}



u64 _resolve_symbol(u64 library,u64 index){
	printf("RESOLVE SYMBOL: %u (got+%u) [%p]\n",index,index+3,library);
	return (u64)__TEST_FUNCTION_STUB;
}



static void _parse_dynamic_section(linker_context_t* ctx){
	ctx->elf_string_table=NULL;
	ctx->elf_hash_table=NULL;
	ctx->elf_symbol_table=NULL;
	ctx->elf_symbol_table_entry_size=0;
	_Bool has_imports=0;
	for (const elf_dyn_t* dyn=ctx->elf_dynamic_section;dyn->d_tag!=DT_NULL;dyn++){
		if (dyn->d_tag==DT_NEEDED){
			has_imports=1;
		}
		else if (dyn->d_tag==DT_HASH){
			ctx->elf_hash_table=dyn->d_un.d_ptr;
		}
		else if (dyn->d_tag==DT_STRTAB){
			ctx->elf_string_table=dyn->d_un.d_ptr;
		}
		else if (dyn->d_tag==DT_SYMTAB){
			ctx->elf_symbol_table=dyn->d_un.d_ptr;
		}
		else if (dyn->d_tag==DT_SYMENT){
			ctx->elf_symbol_table_entry_size=dyn->d_un.d_val;
		}
		else if (dyn->d_tag==DT_PLTGOT){
			printf("PLT GOT: %p\n",dyn->d_un.d_ptr);
			u64* got=dyn->d_un.d_ptr;
			got[1]=0x11223344; // shared object identifier
			got[2]=(u64)_resolve_symbol_trampoline; // shared object resolver
		}
		else if (dyn->d_tag==DT_JMPREL){
			printf("PLT Relocations: %p [R_X86_64_JUMP_SLOT]\n",dyn->d_un.d_ptr);
		}
		else if (dyn->d_tag==DT_PLTREL){
			printf("PLT Relocation type: %s\n",(dyn->d_un.d_val==DT_RELA?"DT_RELA":"DT_REL"));
		}
		else if (dyn->d_tag==DT_PLTRELSZ){
			printf("PLT Relocation section size: %u\n",dyn->d_un.d_val);
		}
	}
	if (ctx->elf_string_table&&ctx->elf_hash_table&&ctx->elf_symbol_table&&ctx->elf_symbol_table_entry_size){
		_load_symbols(ctx,0);
	}
	if (!has_imports){
		return;
	}
	if (!ctx->elf_string_table){
		printf("No string table found in the DYNAMIC section\n");
		return;
	}
	for (const elf_dyn_t* dyn=ctx->elf_dynamic_section;dyn->d_tag!=DT_NULL;dyn++){
		if (dyn->d_tag==DT_NEEDED){
			_load_shared_object(ctx->elf_string_table+dyn->d_un.d_val);
		}
	}
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
	linker_context_t ctx;
	_load_linker_context(base_data,&ctx);
	_find_dynamic_section(&ctx);
	if (!ctx.elf_dynamic_section){
		return ctx.elf_entry_address;
	}
	_parse_dynamic_section(&ctx);
	return ctx.elf_entry_address;
}
