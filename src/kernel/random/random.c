#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/random/random.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "random"



#define BLAKE2B_INIT_STRUCT {{0x6a09e667f3bcc908ull,0xbb67ae8584caa73bull,0x3c6ef372fe94f82bull,0xa54ff53a5f1d36f1ull,0x510e527fade682d1ull,0x9b05688c2b3e6c1full,0x1f83d9abfb41bd6bull,0x5be0cd19137e2179ull}}



typedef struct _BLAKE2B_STATE{
	u64 data[8];
} blake2b_state_t;



static u64 _random_entropy_buffer[16];



void random_init(void){
	LOG("Initializing PRNG...");
	_random_init_entropy_pool();
}



void random_generate(void* buffer,u64 length){
	if (_random_has_entropy()){
		_random_get_entropy(_random_entropy_buffer);
	}
}
