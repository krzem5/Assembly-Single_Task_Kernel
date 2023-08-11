#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/random/random.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "random"



static u32 _random_entropy_buffer[64];



void random_init(void){
	LOG("Initializing PRNG...");
	_random_init_entropy_pool();
}



void random_generate(void* buffer,u64 length){
	_random_get_entropy(_random_entropy_buffer);
}
