#include <kernel/elf/structures.h>
#include <kernel/hash/sha256.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/smm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/signature/signature.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "signature"



#define PROCESS_SECTION(name) hash_sha256_process_chunk(&state,(const void*)(kernel_section_##name##_start()),kernel_section_##name##_end()-kernel_section_##name##_start())

#define SIGNATURE_SECTION_SIZE 4096



static u32 KERNEL_EARLY_WRITE __kernel_signature[8];



void KERNEL_EARLY_EXEC signature_verify_kernel(void){
	LOG("Calculating kernel signature...");
	hash_sha256_state_t state;
	hash_sha256_init(&state);
	PROCESS_SECTION(kernel_ue);
	PROCESS_SECTION(kernel_ur);
	PROCESS_SECTION(kernel_ex);
	PROCESS_SECTION(kernel_nx);
	hash_sha256_finalize(&state);
	u32 mask=0;
	for (u32 i=0;i<8;i++){
		mask|=__builtin_bswap32(state.result[i])^__kernel_signature[i];
		state.result[i]=0;
		__kernel_signature[i]=0;
	}
	if (mask){
		panic("Invalid kernel signature");
	}
}



_Bool signature_verify_module(const char* name,const mmap_region_t* region){
	INFO("Verifying signature of '%s'...",name);
	void* file_base=(void*)(region->rb_node.key);
	const elf_hdr_t* elf_header=file_base;
	const elf_shdr_t* section_header=file_base+elf_header->e_shoff+elf_header->e_shstrndx*elf_header->e_shentsize;
	const char* elf_string_table=file_base+section_header->sh_offset;
	for (u16 i=0;i<elf_header->e_shnum;i++){
		section_header=file_base+elf_header->e_shoff+i*elf_header->e_shentsize;
		if (streq(elf_string_table+section_header->sh_name,".signature")){
			break;
		}
		section_header=NULL;
	}
	if (!section_header||section_header->sh_size!=SIGNATURE_SECTION_SIZE){
		ERROR("Module '%s' is not signed",name);
		return 1;
	}
	u8 module_signature[SIGNATURE_SECTION_SIZE];
	memcpy(module_signature,file_base+section_header->sh_offset,SIGNATURE_SECTION_SIZE);
	memset(file_base+section_header->sh_offset,0,SIGNATURE_SECTION_SIZE);
	hash_sha256_state_t state;
	hash_sha256_init(&state);
	hash_sha256_process_chunk(&state,name,smm_length(name));
	hash_sha256_process_chunk(&state,":",1);
	hash_sha256_process_chunk(&state,file_base,region->length);
	hash_sha256_finalize(&state);
	for (u32 i=0;i<8;i++){
		if (*((const u32*)(module_signature+4*i))!=__builtin_bswap32(state.result[i])){
			goto _error;
		}
		state.result[i]=0;
	}
	memset(module_signature,0,SIGNATURE_SECTION_SIZE);
	ERROR("Module '%s' is not signed",name);
	return 1;
_error:
	ERROR("Module '%s' has an invalid signature",name);
	return 0;
}
