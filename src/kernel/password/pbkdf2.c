#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/password/pbkdf2.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "pbkdf2"



KERNEL_PUBLIC void pbkdf2_compute(const void* password,u32 password_length,const void* salt,u32 salt_length,const prf_t* prf,u32 iterations,void* out,u32 out_length){
	u8* buffer=amm_alloc((prf->output_size>salt_length+4?prf->output_size:salt_length+4));
	memset(out,0,out_length);
	u32 out_offset=0;
	for (u32 i=1;out_offset<out_length;i++){
		u32 chunk_length=out_length-out_offset;
		if (chunk_length>prf->output_size){
			chunk_length=prf->output_size;
		}
		memcpy(buffer,salt,salt_length);
		buffer[salt_length]=i>>24;
		buffer[salt_length+1]=i>>16;
		buffer[salt_length+2]=i>>8;
		buffer[salt_length+3]=i;
		u32 buffer_length=salt_length+4;
		for (u32 j=0;j<iterations;j++){
			prf->callback(password,password_length,buffer,buffer_length,buffer);
			for (u32 k=0;k<chunk_length;k++){
				((u8*)out)[out_offset+k]^=buffer[k];
			}
			buffer_length=prf->output_size;
		}
		out_offset+=chunk_length;
	}
	amm_dealloc(buffer);
}
