#include <kernel/config/config.h>
#include <kernel/elf/elf.h>
#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/initramfs/initramfs.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/mp/process.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#include <kernel/util/string.h>
#include <kernel/util/util.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "module_loader"



#define MODULE_ORDER_FILE "/boot/module/module_order.config"



static void _load_modules_from_order_file(bool early){
	vfs_node_t* file=vfs_lookup(NULL,MODULE_ORDER_FILE,0,0,0);
	if (!file){
		panic("Unable to locate module order file");
	}
	config_tag_t* root_tag=config_load_from_file(file,NULL);
	if (!root_tag){
		panic("Unable to parse module order file");
	}
	if (root_tag->type!=CONFIG_TAG_TYPE_ARRAY){
		panic("Invalid tag type");
	}
	for (u32 i=0;i<root_tag->array->length;i++){
		config_tag_t* tag=root_tag->array->data[i];
		if (tag->type!=CONFIG_TAG_TYPE_NONE&&tag->type!=CONFIG_TAG_TYPE_STRING){
			panic("Invalid tag type");
		}
		if (early!=(tag->type==CONFIG_TAG_TYPE_STRING&&str_equal(tag->string->data,"early"))){
			continue;
		}
#ifdef KERNEL_COVERAGE
		if (tag->type==CONFIG_TAG_TYPE_STRING&&str_equal(tag->string->data,"not-test")){
			continue;
		}
#else
		if (tag->type==CONFIG_TAG_TYPE_STRING&&str_equal(tag->string->data,"test")){
			continue;
		}
#endif
		module_load(tag->name->data);
	}
	config_tag_delete(root_tag);
}



MODULE_PREINIT(){
	LOG("Loading early modules...");
	_load_modules_from_order_file(1);
	LOG("Unloading initramfs...");
	initramfs_unload();
	LOG("Loading modules...");
	_load_modules_from_order_file(0);
	LOG("Loading user shell...");
#ifndef KERNEL_COVERAGE
	error_t handle_id=elf_load("/bin/shell",0,NULL,0,NULL,ELF_LOAD_FLAG_PAUSE_THREAD);
	if (IS_ERROR(handle_id)){
		goto _error;
	}
	handle_t* handle=handle_lookup_and_acquire(handle_id,process_handle_type);
	if (!handle){
		goto _error;
	}
	process_t* process=KERNEL_CONTAINEROF(handle,process_t,handle);
	process->vfs_stdin=vfs_lookup(NULL,"/dev/ser/in",VFS_LOOKUP_FLAG_FOLLOW_LINKS,0,0);
	process->vfs_stdout=vfs_lookup(NULL,"/dev/ser/out",VFS_LOOKUP_FLAG_FOLLOW_LINKS,0,0);
	process->vfs_stderr=vfs_lookup(NULL,"/dev/ser/out",VFS_LOOKUP_FLAG_FOLLOW_LINKS,0,0);
	scheduler_enqueue_thread(process->thread_list.head);
	handle_release(handle);
	return 0;
_error:
	panic("Unable to load user shell");
#endif
	return 0;
}



MODULE_DECLARE(MODULE_FLAG_PREVENT_LOADS);
