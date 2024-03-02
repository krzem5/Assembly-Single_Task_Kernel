#include <kernel/hash/sha256.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/signature/signature.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "signature"



#define PROCESS_SECTION(name) hash_sha256_process_chunk(&state,(const void*)(kernel_section_##name##_start()),kernel_section_##name##_end()-kernel_section_##name##_start())



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
		__kernel_signature[i]=0;
	}
	if (mask){
		panic("Invalid kernel signature");
	}
}
