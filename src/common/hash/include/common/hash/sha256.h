#ifndef _COMMON_HASH_SHA256_H_
#define _COMMON_HASH_SHA256_H_ 1
#include <common/types.h>



typedef struct _HASH_SHA256_STATE{
	union{
		struct{
			u32 a;
			u32 b;
			u32 c;
			u32 d;
			u32 e;
			u32 f;
			u32 g;
			u32 h;
		};
		u32 result32[8];
		u8 result[32];
	};
	u64 length;
	u8 buffer[64];
} hash_sha256_state_t;



void hash_sha256_init(hash_sha256_state_t* out);



void hash_sha256_process_chunk(hash_sha256_state_t* state,const void* chunk,u64 length);



void hash_sha256_finalize(hash_sha256_state_t* state);



#endif
