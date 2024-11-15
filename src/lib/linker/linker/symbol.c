#include <linker/shared_object.h>
#include <linker/symbol.h>
#include <sys/elf/elf.h>
#include <sys/io/io.h>
#include <sys/mp/thread.h>
#include <sys/types.h>



#define NOINLINE_NOFLOAT __attribute__((noinline,target("no-mmx","no-sse","no-sse2")))



static u64 NOINLINE_NOFLOAT _lookup_symbol(const linker_shared_object_t* so,const char* name,_Bool nested){
	if (!so){
		return 0;
	}
	u32 new_hash=5381;
	for (u32 i=0;name[i];i++){
		new_hash=new_hash*33+name[i];
	}
	u32 old_hash=0;
	for (const linker_shared_object_t* end=(nested?NULL:so->next);so!=end;so=so->next){
		if (so==linker_shared_object_root&&(name[0]!='_'||name[1]!='_')){
			continue;
		}
		if (so->dynamic_section.gnu_hash_table){
			u64 mask=so->dynamic_section.gnu_hash_table_bloom_filter[(new_hash>>6)&(so->dynamic_section.gnu_hash_table->maskwords-1)];
			if (!((mask>>(new_hash&63))&(mask>>((new_hash>>so->dynamic_section.gnu_hash_table->shift2)&63))&1)){
				continue;
			}
			u32 i=so->dynamic_section.gnu_hash_table_buckets[new_hash%so->dynamic_section.gnu_hash_table->nbucket];
			if (!i){
				continue;
			}
			const u32* chain=so->dynamic_section.gnu_hash_table_values+i-so->dynamic_section.gnu_hash_table->symndx;
			for (u32 j=0;!j||!(chain[j-1]&1);j++){
				if ((new_hash^chain[j])>>1){
					continue;
				}
				const elf_sym_t* symbol=so->dynamic_section.symbol_table+(i+j)*so->dynamic_section.symbol_table_entry_size;
				const char* symbol_name=so->dynamic_section.string_table+symbol->st_name;
				for (u32 k=0;1;k++){
					if (name[k]!=symbol_name[k]){
						goto _skip_new_entry;
					}
					if (!name[k]){
						return so->image_base+symbol->st_value;
					}
				}
_skip_new_entry:
			}
			continue;
		}
		if (so->dynamic_section.hash_table){
			if (!old_hash){
				for (u32 i=0;name[i];i++){
					old_hash=(old_hash<<4)+name[i];
					old_hash^=(old_hash>>24)&0xf0;
				}
				old_hash&=0x0fffffff;
			}
			for (u32 i=so->dynamic_section.hash_table->data[old_hash%so->dynamic_section.hash_table->nbucket];i;i=so->dynamic_section.hash_table->data[i+so->dynamic_section.hash_table->nbucket]){
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
	}
	return 0;
}



u64 NOINLINE_NOFLOAT linker_symbol_lookup_by_name(const char* name){
	return _lookup_symbol(linker_shared_object_root,name,1);
}



u64 linker_symbol_lookup_by_name_in_shared_object(const linker_shared_object_t* so,const char* name){
	return _lookup_symbol(so,name,0);
}



u64 NOINLINE_NOFLOAT linker_symbol_resolve_plt(const linker_shared_object_t* so,u64 index){
	const elf_rela_t* relocation=so->dynamic_section.plt_relocations+index*so->dynamic_section.plt_relocation_entry_size;
	if ((relocation->r_info&0xffffffff)!=R_X86_64_JUMP_SLOT){
		sys_io_print("Wrong plt relocation type '%u' in shared object '%s'\n",(u32)(relocation->r_info),so->path);
		sys_thread_stop(0,NULL);
		return 0;
	}
	const elf_sym_t* symbol=so->dynamic_section.symbol_table+(relocation->r_info>>32)*so->dynamic_section.symbol_table_entry_size;
	const char* symbol_name=so->dynamic_section.string_table+symbol->st_name;
	u64 resolved_symbol=linker_symbol_lookup_by_name(symbol_name);
	if (!resolved_symbol){
		sys_io_print("Unable to resolve symbol '%s' in shared object '%s'\n",symbol_name,so->path);
		sys_thread_stop(0,NULL);
		return 0;
	}
	if (so->dynamic_section.plt_relocation_entry_size==sizeof(elf_rela_t)){
		resolved_symbol+=relocation->r_addend;
	}
	*((u64*)(so->image_base+relocation->r_offset))=resolved_symbol;
	return resolved_symbol;
}
