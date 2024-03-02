#ifndef _KERNEL_MODULE_MODULE_H_
#define _KERNEL_MODULE_MODULE_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/memory/smm.h>
#include <kernel/mmap/mmap.h>
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



typedef struct _MODULE{
	handle_t handle;
	string_t* name;
	const struct _MODULE_DESCRIPTOR* descriptor;
	mmap_region_t* region;
#ifdef KERNEL_COVERAGE
	u64 gcov_info_base;
	u64 gcov_info_size;
#endif
	u32 flags;
	u32 state;
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



_Bool module_unload(module_t* module);



module_t* module_lookup(const char* name);



#endif
