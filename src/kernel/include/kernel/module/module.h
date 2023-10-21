#ifndef _KERNEL_MODULE_MODULE_H_
#define _KERNEL_MODULE_MODULE_H_ 1
#include <kernel/types.h>
#include <kernel/vfs/node.h>



#define MODULE_DECLARE(name,init_callback,deinit_callback) \
	static const module_descriptor_t __attribute__((used,section(".module"))) _module_descriptor={ \
		#name, \
		(init_callback), \
		(deinit_callback) \
	}



typedef struct _MODULE_DESCRIPTOR{
	const char* name;
	_Bool (*init_callback)(void);
	void (*deinit_callback)(void);
} module_descriptor_t;



_Bool module_load(vfs_node_t* node);



#endif
