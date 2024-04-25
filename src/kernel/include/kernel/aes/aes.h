#ifndef _KERNEL_AES_AES_H_
#define _KERNEL_AES_AES_H_ 1
#include <kernel/types.h>



#define AES_FLAG_ENCRYPTION 1
#define AES_FLAG_DECRYPTION 2



typedef struct _AES_STATE{
	u32* encryption_key;
	u32* decryption_key;
	u32 rounds;
} aes_state_t;



void aes_init(const void* key,u32 key_length,u32 flags,aes_state_t* out);



void aes_deinit(aes_state_t* state);



void aes_encrypt_block(const aes_state_t* state,const void* data,void* out);



void aes_decrypt_block(const aes_state_t* state,const void* data,void* out);



void aes_cbc_process(const void* key,u32 key_length,const void* iv,u32 iv_length,u32 flags,const void* data,u32 data_length,void* out);



#endif
