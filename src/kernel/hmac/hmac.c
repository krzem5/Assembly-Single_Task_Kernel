#include <kernel/hmac/hmac.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/util.h>



#define BUFFER_SIZE 265



KERNEL_PUBLIC void hmac_compute(const void* key,u32 key_length,const void* message,u32 message_length,const hmac_hash_function_t* func,void* out){
	if (func->block_size+func->output_size>BUFFER_SIZE){
		panic("hmac_compute: buffer too small");
	}
	u8 buffer[BUFFER_SIZE];
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
	mem_fill(buffer,sizeof(buffer),0);
}
