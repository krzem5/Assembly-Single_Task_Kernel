#include <kernel/memory/amm.h>
#include <kernel/rsa/rsa.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



static void _calculate_mu(rsa_state_t* state){
	//
}



rsa_number_t* rsa_number_create(const rsa_state_t* state){
	rsa_number_t* out=amm_alloc(sizeof(rsa_number_t)+state->max_number_length*sizeof(u32));
	out->length=0;
	out->capacity=state->max_number_length;
	memset(out->data,0,state->max_number_length*sizeof(u32));
	return out;
}



void rsa_number_delete(rsa_number_t* number){
	memset(number->data,0,number->capacity*sizeof(u32));
	number->length=0;
	number->capacity=0;
	amm_dealloc(number);
}



void rsa_state_init(const u32* modulus,u32 modulus_bit_length,rsa_state_t* out){
	u32 modulus_length=(modulus_bit_length+31)>>5;
	out->max_number_length=(modulus_length<<1)+1;
	out->modulus_bit_length=modulus_bit_length;
	out->public_key=NULL;
	out->private_key=NULL;
	out->modulus=rsa_number_create(out);
	out->modulus->length=modulus_length;
	memcpy(out->modulus->data,modulus,modulus_length*sizeof(u32));
	out->modulus->data[modulus_bit_length>>5]&=(1ull<<(modulus_bit_length&31))-1;
	_calculate_mu(out);
}



void rsa_state_deinit(rsa_state_t* state);



void rsa_state_process(const rsa_state_t* state,rsa_number_t* value,u32 key,rsa_number_t* out);
