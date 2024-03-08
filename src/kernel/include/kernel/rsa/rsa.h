#ifndef _KERNEL_RSA_RSA_H_
#define _KERNEL_RSA_RSA_H_ 1
#include <kernel/types.h>



#define RSA_PUBLIC_KEY 1
#define RSA_PRIVATE_KEY 2



typedef struct _RSA_NUMBER{
	u32 length;
	u32 capacity;
	u32 data[];
} rsa_number_t;



typedef struct _RSA_STATE{
	u32 max_number_length;
	u32 modulus_bit_length;
	rsa_number_t* public_key;
	rsa_number_t* private_key;
	rsa_number_t* modulus;
	rsa_number_t* _mu;
} rsa_state_t;



rsa_number_t* rsa_number_create(const rsa_state_t* state);



rsa_number_t* rsa_number_create_from_bytes(const rsa_state_t* state,const u32* data,u32 length);



void rsa_number_delete(rsa_number_t* number);



void rsa_state_init(const u32* modulus,u32 modulus_bit_length,rsa_state_t* out);



void rsa_state_deinit(rsa_state_t* state);



void rsa_state_process(const rsa_state_t* state,rsa_number_t* value,u32 key,rsa_number_t* out);



#endif
