#ifndef _KERNEL_MODULE_MODULE_H_
#define _KERNEL_MODULE_MODULE_H_ 1
#include <kernel/types.h>
#include <kernel/vfs/node.h>



#define MODULE_DECLARE(name,init_callback,deinit_callback) \
	static const module_descriptor_t __attribute__((used,section(".module"))) _module_descriptor={ \
		(name), \
		(init_callback), \
		(deinit_callback) \
	}



typedef struct _MODULE_ADDRESS_REGION{
	u64 base;
	u64 size;
} module_address_region_t;



typedef struct _MODULE{
	const struct _MODULE_DESCRIPTOR* descriptor;
	module_address_region_t ex_region;
	module_address_region_t nx_region;
	module_address_region_t rw_region;
} module_t;



typedef struct _MODULE_DESCRIPTOR{
	const char* name;
	_Bool (*init_callback)(module_t*);
	void (*deinit_callback)(module_t*);
} module_descriptor_t;



_Bool module_load(vfs_node_t* node);



#endif
