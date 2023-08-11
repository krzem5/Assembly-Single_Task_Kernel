#ifndef _KERNEL_RANDOM_RANDOM_H_
#define _KERNEL_RANDOM_RANDOM_H_ 1
#include <kernel/types.h>



void random_init(void);



void random_generate(void* buffer,u64 length);



void _random_init_entropy_pool(void);



_Bool _random_has_entropy(void);



void _random_get_entropy(void* buffer);



#endif
