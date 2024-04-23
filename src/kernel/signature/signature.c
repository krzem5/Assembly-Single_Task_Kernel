#include <kernel/elf/structures.h>
#include <kernel/hash/sha256.h>
#include <kernel/kernel.h>
#include <kernel/keyring/keyring.h>
#include <kernel/log/log.h>
#include <kernel/memory/smm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/mp/thread.h>
#include <kernel/rsa/rsa.h>
#include <kernel/signature/signature.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/string.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "signature"



#define PROCESS_SECTION(name) hash_sha256_process_chunk(&state,(const void*)(kernel_section_##name##_start()),kernel_section_##name##_end()-kernel_section_##name##_start())

#define SIGNATURE_SECTION_SIZE 4096

#define SIGNATURE_KEY_NAME_LENGTH 64



static volatile u32 KERNEL_EARLY_WRITE __kernel_signature[8];
static bool _signature_is_kernel_tainted=0;
static bool _signature_require_signatures=0;



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
		mask|=state.result32[i]^__kernel_signature[i];
		state.result[i]=0;
		__kernel_signature[i]=0;
	}
	if (mask){
		panic("Invalid kernel signature");
	}
}



bool signature_verify_module(const char* name,const mmap_region_t* region,bool* is_tainted){
	*is_tainted=1;
	INFO("Verifying signature of '%s'...",name);
	void* file_base=(void*)(region->rb_node.key);
	const elf_hdr_t* elf_header=file_base;
	const elf_shdr_t* section_header=file_base+elf_header->e_shoff+elf_header->e_shstrndx*elf_header->e_shentsize;
	const char* elf_string_table=file_base+section_header->sh_offset;
	for (u16 i=0;i<elf_header->e_shnum;i++){
		section_header=file_base+elf_header->e_shoff+i*elf_header->e_shentsize;
		if (str_equal(elf_string_table+section_header->sh_name,".signature")){
			break;
		}
		section_header=NULL;
	}
	if (!section_header||section_header->sh_size!=SIGNATURE_SECTION_SIZE||*((const u8*)(file_base+section_header->sh_offset+SIGNATURE_KEY_NAME_LENGTH-1))){
_signature_error:
		ERROR("Module '%s' is not signed",name);
		if (!_signature_is_kernel_tainted){
			ERROR("Kernel tainted");
			_signature_is_kernel_tainted=1;
		}
		return 1;
	}
	keyring_key_t* key=keyring_search(keyring_module_signature,file_base+section_header->sh_offset,KEYRING_SEARCH_FLAG_BYPASS_ACL);
	if (!key||key->type!=KEYRING_KEY_TYPE_RSA){
		goto _signature_error;
	}
	rsa_number_t* value=rsa_number_create_from_bytes(&(key->data.rsa.state),file_base+section_header->sh_offset+SIGNATURE_KEY_NAME_LENGTH,(SIGNATURE_SECTION_SIZE-SIGNATURE_KEY_NAME_LENGTH)/sizeof(u32));
	mem_fill(file_base+section_header->sh_offset,SIGNATURE_SECTION_SIZE,0);
	if (!keyring_key_process_rsa(key,value,value)){
		rsa_number_delete(value);
		goto _signature_error;
	}
	hash_sha256_state_t state;
	hash_sha256_init(&state);
	hash_sha256_process_chunk(&state,name,smm_length(name));
	hash_sha256_process_chunk(&state,":",1);
	hash_sha256_process_chunk(&state,file_base,region->length);
	hash_sha256_finalize(&state);
	u32 mask=0;
	for (u32 i=0;i<8;i++){
		mask|=state.result32[i]^value->data[i];
		state.result[i]=0;
	}
	rsa_number_delete(value);
	if (mask){
		ERROR("Module '%s' has an invalid signature",name);
	}
	else{
		*is_tainted=0;
	}
	return !mask;
}



bool signature_verify_user(const char* name,const mmap_region_t* region){
	INFO("Verifying signature of '%s'...",name);
	void* file_base=(void*)(region->rb_node.key);
	const elf_hdr_t* elf_header=file_base;
	const elf_shdr_t* section_header=file_base+elf_header->e_shoff+elf_header->e_shstrndx*elf_header->e_shentsize;
	const char* elf_string_table=file_base+section_header->sh_offset;
	for (u16 i=0;i<elf_header->e_shnum;i++){
		section_header=file_base+elf_header->e_shoff+i*elf_header->e_shentsize;
		if (str_equal(elf_string_table+section_header->sh_name,".signature")){
			break;
		}
		section_header=NULL;
	}
	if (!section_header||section_header->sh_size!=SIGNATURE_SECTION_SIZE||*((const u8*)(file_base+section_header->sh_offset+SIGNATURE_KEY_NAME_LENGTH-1))){
_signature_error:
		if (_signature_require_signatures){
			goto _invalid_signature;
		}
		WARN("Executable '%s' is not signed",name);
		return 1;
	}
	keyring_key_t* key=keyring_search(keyring_user_signature,file_base+section_header->sh_offset,KEYRING_SEARCH_FLAG_BYPASS_ACL);
	if (!key||key->type!=KEYRING_KEY_TYPE_RSA){
		goto _signature_error;
	}
	rsa_number_t* value=rsa_number_create_from_bytes(&(key->data.rsa.state),file_base+section_header->sh_offset+SIGNATURE_KEY_NAME_LENGTH,(SIGNATURE_SECTION_SIZE-SIGNATURE_KEY_NAME_LENGTH)/sizeof(u32));
	mem_fill(file_base+section_header->sh_offset,SIGNATURE_SECTION_SIZE,0);
	if (!keyring_key_process_rsa(key,value,value)){
		rsa_number_delete(value);
		goto _signature_error;
	}
	hash_sha256_state_t state;
	hash_sha256_init(&state);
	hash_sha256_process_chunk(&state,file_base,region->length);
	hash_sha256_finalize(&state);
	u32 mask=0;
	for (u32 i=0;i<8;i++){
		mask|=state.result32[i]^value->data[i];
		state.result[i]=0;
	}
	rsa_number_delete(value);
	if (mask){
_invalid_signature:
		ERROR("Executable '%s' has an invalid signature",name);
	}
	return !mask;
}



KERNEL_PUBLIC bool signature_is_kernel_tainted(void){
	return _signature_is_kernel_tainted;
}



void syscall_signature_verify(const char* name,void* data,u64 size){
	u64 name_length=syscall_get_string_length((const char*)name);
	mmap_region_t* region=mmap_lookup(THREAD_DATA->process->mmap,(u64)data);
	if (!name_length||name_length>255||!size||!region||region->rb_node.key!=(u64)data||region->length!=size){
		ERROR("syscall_signature_verify: invalid arguments");
		goto _error;
	}
	char buffer[256];
	mem_copy(buffer,(const char*)name,name_length);
	buffer[name_length]=0;
	if (!signature_verify_user(buffer,region)){
		goto _error;
	}
	return;
_error:
	*((u8*)0)=0;
}
