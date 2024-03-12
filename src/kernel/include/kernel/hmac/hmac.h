#ifndef _KERNEL_HMAC_HMAC_H_
#define _KERNEL_HMAC_HMAC_H_ 1
#include <kernel/types.h>



typedef void (*hmac_hash_function_callback_t)(const void*,u32,const void*,u32,void*);



typedef struct _HMAC_HASH_FUNCTION{
	hmac_hash_function_callback_t callback;
	u32 block_size;
	u32 output_size;
} hmac_hash_function_t;



void hmac_compute(const void* key,u32 key_length,const void* message,u32 message_length,const hmac_hash_function_t* func,void* out);



#endif
