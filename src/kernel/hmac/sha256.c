#include <kernel/hash/sha256.h>
#include <kernel/hmac/hmac.h>
#include <kernel/types.h>



static void _hmac_sha256_callback(const void* data1,u32 length1,const void* data2,u32 length2,void* buffer){
	hash_sha256_state_t state;
	hash_sha256_init(&state);
	hash_sha256_process_chunk(&state,data1,length1);
	hash_sha256_process_chunk(&state,data2,length2);
	hash_sha256_finalize(&state);
	u32* out=buffer;
	for (u32 i=0;i<8;i++){
		out[i]=__builtin_bswap32(state.result[i]);
	}
}



static const hmac_hash_function_t _hmac_sha256_function_data={
	_hmac_sha256_callback,
	64,
	32
};



const hmac_hash_function_t* hmac_sha256_function=&_hmac_sha256_function_data;
