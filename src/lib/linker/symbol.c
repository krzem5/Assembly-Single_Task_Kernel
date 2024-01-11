#include <linker/shared_object.h>
#include <sys/elf/elf.h>
#include <sys/io/io.h>
#include <sys/types.h>



#define NOFLOAT __attribute__((target("no-mmx","no-sse","no-sse2")))



u64 NOFLOAT symbol_lookup_by_name(const char* name){
	u32 hash=0;
	for (const char* tmp=name;tmp[0];tmp++){
		hash=(hash<<4)+tmp[0];
		hash^=(hash>>24)&0xf0;
	}
	hash&=0x0fffffff;
	for (const shared_object_t* so=shared_object_root;so;so=so->next){
		if (!so->dynamic_section.hash_table){
			continue;
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
	}
	return 0;
}



u64 NOFLOAT symbol_resolve_plt(shared_object_t* so,u64 index){
	const elf_rela_t* relocation=so->dynamic_section.plt_relocations+index*so->dynamic_section.plt_relocation_entry_size;
	if ((relocation->r_info&0xffffffff)!=R_X86_64_JUMP_SLOT){
		return 0;
	}
	const elf_sym_t* symbol=so->dynamic_section.symbol_table+(relocation->r_info>>32)*so->dynamic_section.symbol_table_entry_size;
	u64 resolved_symbol=symbol_lookup_by_name(so->dynamic_section.string_table+symbol->st_name);
	if (!resolved_symbol){
		return 0;
	}
	if (so->dynamic_section.plt_relocation_entry_size==sizeof(elf_rela_t)){
		resolved_symbol+=relocation->r_addend;
	}
	*((u64*)(so->image_base+relocation->r_offset))=resolved_symbol;
	return resolved_symbol;
}
