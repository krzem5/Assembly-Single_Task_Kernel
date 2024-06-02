#ifndef _KERNEL_PASSWORD_PBKDF2_H_
#define _KERNEL_PASSWORD_PBKDF2_H_ 1
#include <kernel/types.h>



typedef void (*prf_callback_t)(const void*,u32,const void*,u32,void*);



typedef struct _PRF{
	prf_callback_t callback;
	u32 output_size;
} prf_t;



extern const prf_t*const pbkdf2_prf_hmac_sha256;



void pbkdf2_compute(const void* password,u32 password_length,const void* salt,u32 salt_length,const prf_t* prf,u32 iterations,void* out,u32 out_length);



#endif
