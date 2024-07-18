#ifndef _KERNEL_MODULE_MODULE_H_
#define _KERNEL_MODULE_MODULE_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/memory/smm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/notification/notification.h>
#include <kernel/types.h>
#include <kernel/vfs/node.h>



#define MODULE_STATE_UNKNOWN 0
#define MODULE_STATE_LOADING 1
#define MODULE_STATE_LOADED 2
#define MODULE_STATE_UNLOADING 3
#define MODULE_STATE_UNLOADED 4

#define MODULE_FLAG_PREVENT_LOADS 1
#define MODULE_FLAG_NO_SIGNATURE 2
#define MODULE_FLAG_TAINTED 4
#define _MODULE_FLAG_EARLY_UNLOAD 8

#define MODULE_DECLARE(flags) \
	extern u64 __module_section_init_start[1]; \
	extern u64 __module_section_init_end[1]; \
	extern u64 __module_section_postinit_start[1]; \
	extern u64 __module_section_postinit_end[1]; \
	extern u64 __module_section_postpostinit_start[1]; \
	extern u64 __module_section_postpostinit_end[1]; \
	extern u64 __module_section_deinit_start[1]; \
	extern u64 __module_section_deinit_end[1]; \
	extern u64 __module_section_gcov_info_start[1]; \
	extern u64 __module_section_gcov_info_end[1]; \
	module_t* module_self=NULL; \
	static const module_descriptor_t KERNEL_EARLY_READ __attribute__((used)) __module_header={ \
		(flags), \
		&module_self, \
		(u64)(__module_section_init_start), \
		(u64)(__module_section_init_end), \
		(u64)(__module_section_postinit_start), \
		(u64)(__module_section_postinit_end), \
		(u64)(__module_section_postpostinit_start), \
		(u64)(__module_section_postpostinit_end), \
		(u64)(__module_section_deinit_start), \
		(u64)(__module_section_deinit_end), \
		(u64)(__module_section_gcov_info_start), \
		(u64)(__module_section_gcov_info_end) \
	}; \
	static const u8 __attribute__((used,section(".signature"))) _module_signature[(((flags)&MODULE_FLAG_NO_SIGNATURE)?0:4096)]

#define MODULE_INIT() static KERNEL_EARLY_EXEC void _KERNEL_INITIALIZER_NAME(__init_)(void);static void* __attribute__((section(".module_init"),used)) __init_ptr=_KERNEL_INITIALIZER_NAME(__init_);static KERNEL_EARLY_EXEC __attribute__((constructor)) void _KERNEL_INITIALIZER_NAME(__init_)(void)
#define MODULE_POSTINIT() static KERNEL_EARLY_EXEC void _KERNEL_INITIALIZER_NAME(__postinit_)(void);static void* __attribute__((section(".module_postinit"),used)) __postinit_ptr=_KERNEL_INITIALIZER_NAME(__postinit_);static KERNEL_EARLY_EXEC __attribute__((constructor)) void _KERNEL_INITIALIZER_NAME(__postinit_)(void)
#define MODULE_POSTPOSTINIT() static KERNEL_EARLY_EXEC void _KERNEL_INITIALIZER_NAME(__postpostinit_)(void);static void* __attribute__((section(".module_postpostinit"),used)) __postpostinit_ptr=_KERNEL_INITIALIZER_NAME(__postpostinit_);static KERNEL_EARLY_EXEC __attribute__((constructor)) void _KERNEL_INITIALIZER_NAME(__postpostinit_)(void)
#define MODULE_DEINIT() static void __deinit(void);static void* __attribute__((section(".module_deinit"),used)) __deinit_ptr=__deinit;static void __deinit(void)



#define MODULE_LOAD_NOTIFICATION 0x00000001
#define MODULE_UNLOAD_NOTIFICATION 0x00000002



typedef struct _MODULE{
	handle_t handle;
	string_t* name;
	mmap_region_t* region;
	u64 deinit_array_base;
	u64 deinit_array_size;
#ifdef KERNEL_COVERAGE
	u64 gcov_info_base;
	u64 gcov_info_size;
#endif
	u32 flags;
	u32 state;
} module_t;



typedef struct _MODULE_DESCRIPTOR{
	u32 flags;
	module_t** module_self_ptr;
	u64 init_start;
	u64 init_end;
	u64 postinit_start;
	u64 postinit_end;
	u64 postpostinit_start;
	u64 postpostinit_end;
	u64 deinit_start;
	u64 deinit_end;
	u64 gcov_info_start;
	u64 gcov_info_end;
} module_descriptor_t;



typedef struct _MODULE_LOAD_NOTIFICATION_DATA{
	handle_id_t module_handle;
	char name[];
} module_load_notification_data_t;



typedef struct _MODULE_UNLOAD_NOTIFICATION_DATA{
	handle_id_t module_handle;
	char name[];
} module_unload_notification_data_t;



extern handle_type_t module_handle_type;
extern module_t* module_self;
extern notification_dispatcher_t* module_notification_dispatcher;



module_t* module_load(const char* name);



bool module_unload(module_t* module);



module_t* module_lookup(const char* name);



#endif
