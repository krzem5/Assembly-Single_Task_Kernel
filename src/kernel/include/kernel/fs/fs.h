#ifndef _KERNEL_FS_FS_H_
#define _KERNEL_FS_FS_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/fs/_fs_types.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/lock.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>



typedef struct _FILESYSTEM_DESCRIPTOR{
	const char* name;
	void (*deinit_callback)(filesystem_t*);
	filesystem_t* (*load_callback)(partition_t*);
	handle_t handle;
} filesystem_descriptor_t;



extern handle_type_t HANDLE_TYPE_FS;
extern handle_type_t HANDLE_TYPE_FS_DESCRIPTOR;



void fs_register_descriptor(filesystem_descriptor_t* descriptor);



void fs_unregister_descriptor(filesystem_descriptor_t* descriptor);



filesystem_t* fs_create(filesystem_descriptor_t* descriptor);



filesystem_t* fs_load(partition_t* partition);



#endif
