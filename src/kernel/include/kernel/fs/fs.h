#ifndef _KERNEL_FS_FS_H_
#define _KERNEL_FS_FS_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/fs/_fs_types.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/lock.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>



#define FILESYSTEM_DECLARE_TYPE(name,deinit_callback,load_callback) \
	filesystem_type_t FILESYSTEM_TYPE_##name; \
	static const filesystem_descriptor_t _filesystem_descriptor_##name={ \
		#name, \
		&(FILESYSTEM_TYPE_##name), \
		(deinit_callback), \
		(load_callback) \
	}; \
	static const filesystem_descriptor_t*const __attribute__((used,section(".filesystem"))) _filesystem_descriptor_ptr_##name=&_filesystem_descriptor_##name;



typedef struct _FILESYSTEM_DESCRIPTOR{
	const char* name;
	filesystem_type_t* var;
	void (*deinit_callback)(filesystem_t*);
	filesystem_t* (*load_callback)(partition_t*);
} filesystem_descriptor_t;



extern handle_type_t HANDLE_TYPE_FS;



void fs_init(void);



filesystem_t* fs_create(filesystem_type_t type);



filesystem_t* fs_load(partition_t* partition);



#endif
