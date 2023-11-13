#include <core/elf.h>
#include <core/fd.h>
#include <core/io.h>
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



static void _load_symbols(linker_context_t* ctx){
	for (u32 i=0;i<ctx->elf_hash_table->nbucket;i++){
		for (u32 j=ctx->elf_hash_table->data[i];1;j=ctx->elf_hash_table->data[ctx->elf_hash_table->nbucket+j]){
			const elf_sym_t* symbol=ctx->elf_symbol_table+j*ctx->elf_symbol_table_entry_size;
			if (symbol->st_shndx==SHN_UNDEF){
				break;
			}
			symbol_table_add(ctx->elf_string_table+symbol->st_name,symbol->st_value);
		}
	}
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
	}
	if (ctx->elf_string_table&&ctx->elf_hash_table&&ctx->elf_symbol_table&&ctx->elf_symbol_table_entry_size){
		_load_symbols(ctx);
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
			char buffer[4096];
			s64 fd=resolve_library_name(ctx->elf_string_table+dyn->d_un.d_val,buffer,4096);
			if (!fd){
				printf("Unable to find library '%s'\n",ctx->elf_string_table+dyn->d_un.d_val);
				return;
			}
			printf("Found library: %s [%ld]\n",buffer,fd);
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
