#include <linker/shared_object.h>
#include <linker/symbol.h>
#include <sys/elf/elf.h>
#include <sys/io/io.h>
#include <sys/mp/thread.h>
#include <sys/types.h>



#define NOFLOAT __attribute__((target("no-mmx","no-sse","no-sse2")))



static u32 NOFLOAT _calculate_hash(const char* name){
	u32 out=0;
	for (;name[0];name++){
		out=(out<<4)+name[0];
		out^=(out>>24)&0xf0;
	}
	return out&0x0fffffff;
}



static u64 NOFLOAT _lookup_symbol(const shared_object_t* so,u32 hash,const char* name){
	if (!so->dynamic_section.hash_table||!so->dynamic_section.hash_table->nbucket){
		return 0;
	}
	for (u32 i=so->dynamic_section.hash_table->data[hash%so->dynamic_section.hash_table->nbucket];i;i=so->dynamic_section.hash_table->data[i+so->dynamic_section.hash_table->nbucket]){
		const elf_sym_t* symbol=so->dynamic_section.symbol_table+i*so->dynamic_section.symbol_table_entry_size;
		if (symbol->st_shndx==SHN_UNDEF){
			continue;
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
	return 0;
}



u64 NOFLOAT symbol_lookup_by_name(const char* name){
	u32 hash=_calculate_hash(name);
	for (const shared_object_t* so=shared_object_root;so;so=so->next){
		u64 address=_lookup_symbol(so,hash,name);
		if (address){
			return address;
		}
	}
	return 0;
}



u64 symbol_lookup_by_name_in_shared_object(const shared_object_t* so,const char* name){
	return _lookup_symbol(so,_calculate_hash(name),name);
}



u64 NOFLOAT symbol_resolve_plt(const shared_object_t* so,u64 index){
	const elf_rela_t* relocation=so->dynamic_section.plt_relocations+index*so->dynamic_section.plt_relocation_entry_size;
	if ((relocation->r_info&0xffffffff)!=R_X86_64_JUMP_SLOT){
		sys_io_print("Wrong plt relocation type '%u' in shared object '%s'\n",(u32)(relocation->r_info),so->path);
		sys_thread_stop(0);
		return 0;
	}
	const elf_sym_t* symbol=so->dynamic_section.symbol_table+(relocation->r_info>>32)*so->dynamic_section.symbol_table_entry_size;
	const char* symbol_name=so->dynamic_section.string_table+symbol->st_name;
	u64 resolved_symbol=symbol_lookup_by_name(symbol_name);
	if (!resolved_symbol){
		sys_io_print("Unable to resolve symbol '%s' in shared object '%s'\n",symbol_name,so->path);
		sys_thread_stop(0);
		return 0;
	}
	if (so->dynamic_section.plt_relocation_entry_size==sizeof(elf_rela_t)){
		resolved_symbol+=relocation->r_addend;
	}
	*((u64*)(so->image_base+relocation->r_offset))=resolved_symbol;
	return resolved_symbol;
}
