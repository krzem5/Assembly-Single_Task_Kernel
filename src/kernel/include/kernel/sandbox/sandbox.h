#ifndef _KERNEL_SANDBOX_SANDBOX_H_
#define _KERNEL_SANDBOX_SANDBOX_H_ 1
#include <kernel/types.h>



#define SANDBOX_NAME_LENGTH 64

#define SANDBOX_DECLARE_TYPE(name) \
	sandbox_flag_t SANDBOX_FLAG_##name; \
	static const sandbox_descriptor_t _sandbox_descriptor_##name={ \
		#name, \
		&(SANDBOX_FLAG_##name) \
	}; \
	static const sandbox_descriptor_t* __attribute__((used,section(".sandbox"))) _sandbox_descriptor_ptr_##name=&_sandbox_descriptor_##name;



typedef u16 sandbox_flag_t;



typedef struct _SANDBOX{
	_Atomic u64 bitmap[0];
} sandbox_t;



typedef struct _SANDBOX_DESCRIPTOR{
	char name[SANDBOX_NAME_LENGTH];
	sandbox_flag_t* var;
} sandbox_descriptor_t;



void sandbox_init(void);



sandbox_t* sandbox_new(void);



void sandbox_delete(sandbox_t* sandbox);



_Bool sandbox_get(const sandbox_t* sandbox,sandbox_flag_t flag);



_Bool sandbox_set(sandbox_t* sandbox,sandbox_flag_t flag,_Bool state);



#endif
