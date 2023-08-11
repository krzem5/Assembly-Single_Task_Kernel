#include <kernel/log/log.h>
#include <kernel/random/random.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "random"



void random_init(void){
	LOG("Initializing PRNG...");
	_random_init_entropy_pool();
}
