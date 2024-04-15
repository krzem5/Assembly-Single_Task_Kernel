#include <kernel/hmac/hmac.h>
#include <kernel/hmac/sha256.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/password/pbkdf2.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#define KERNEL_LOG_NAME "pbkdf2"



static void _pbkdf2_hmac_sha256_callback(const void* data1,u32 length1,const void* data2,u32 length2,void* buffer){
	hmac_compute(data1,length1,data2,length2,hmac_sha256_function,buffer);
}



static const prf_t _pbkdf2_prf_hmac_sha256_data={
	_pbkdf2_hmac_sha256_callback,
	32
};



KERNEL_PUBLIC const prf_t* pbkdf2_prf_hmac_sha256=&_pbkdf2_prf_hmac_sha256_data;



KERNEL_PUBLIC void pbkdf2_compute(const void* password,u32 password_length,const void* salt,u32 salt_length,const prf_t* prf,u32 iterations,void* out,u32 out_length){
	u8* buffer=amm_alloc((prf->output_size>salt_length+4?prf->output_size:salt_length+4));
	mem_fill(out,out_length,0);
	u32 out_offset=0;
	for (u32 i=1;out_offset<out_length;i++){
		u32 chunk_length=out_length-out_offset;
		if (chunk_length>prf->output_size){
			chunk_length=prf->output_size;
		}
		mem_copy(buffer,salt,salt_length);
		*((u32*)(buffer+salt_length))=__builtin_bswap32(i);
		for (u32 j=0;j<iterations;j++){
			prf->callback(password,password_length,buffer,(j?prf->output_size:salt_length+4),buffer);
			for (u32 k=0;k<chunk_length;k++){
				((u8*)out)[out_offset+k]^=buffer[k];
			}
		}
		out_offset+=chunk_length;
	}
	amm_dealloc(buffer);
}
