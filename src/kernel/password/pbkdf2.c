#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/password/pbkdf2.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "pbkdf2"



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
