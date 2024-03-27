#include <kernel/hmac/hmac.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "hmac"



KERNEL_PUBLIC void hmac_compute(const void* key,u32 key_length,const void* message,u32 message_length,const hmac_hash_function_t* func,void* out){
	u8* buffer=amm_alloc(func->block_size+func->output_size);
	if (key_length>func->block_size){
		func->callback(key,key_length,NULL,0,buffer);
		key_length=func->output_size;
	}
	else{
		mem_copy(buffer,key,key_length);
	}
	for (u32 i=0;i<key_length;i++){
		buffer[i]^=0x36;
	}
	for (u32 i=key_length;i<func->block_size;i++){
		buffer[i]=0x36;
	}
	func->callback(buffer,func->block_size,message,message_length,buffer+func->block_size);
	for (u32 i=0;i<func->block_size;i++){
		buffer[i]^=0x6a;
	}
	func->callback(buffer,func->block_size+func->output_size,NULL,0,out);
	for (u32 i=0;i<func->block_size+func->output_size;i++){
		buffer[i]=0;
	}
	amm_dealloc(buffer);
}
