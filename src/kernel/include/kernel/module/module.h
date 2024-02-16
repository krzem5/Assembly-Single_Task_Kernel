#ifndef _KERNEL_MODULE_MODULE_H_
#define _KERNEL_MODULE_MODULE_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/memory/smm.h>
#include <kernel/types.h>
#include <kernel/vfs/node.h>



#define MODULE_STATE_UNKNOWN 0
#define MODULE_STATE_LOADING 1
#define MODULE_STATE_LOADED 2
#define MODULE_STATE_UNLOADING 3
#define MODULE_STATE_UNLOADED 4

#define MODULE_FLAG_PREVENT_LOADS 1

#define MODULE_DECLARE(init_callback,deinit_callback,flags) \
	module_t* module_self=NULL; \
	static const module_descriptor_t __attribute__((used,section(".module"))) _module_descriptor={ \
		(init_callback), \
		(deinit_callback), \
		(flags), \
		&module_self \
	}



typedef struct _MODULE_ADDRESS_REGION{
	u64 base;
	u64 size;
} module_address_range_t;



typedef struct _MODULE{
	handle_t handle;
	string_t* name;
	const struct _MODULE_DESCRIPTOR* descriptor;
	module_address_range_t ex_region;
	module_address_range_t nx_region;
	module_address_range_t rw_region;
#if KERNEL_COVERAGE_ENABLED
	module_address_range_t gcov_info;
#endif
	u32 flags;
	u8 state;
} module_t;



typedef struct _MODULE_DESCRIPTOR{
	_Bool (*init_callback)(module_t*);
	void (*deinit_callback)(module_t*);
	u32 flags;
	module_t** module_self_ptr;
} module_descriptor_t;



extern handle_type_t module_handle_type;
extern module_t* module_self;



module_t* module_load(const char* name);



void module_unload(module_t* module);



#endif
