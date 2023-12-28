#include <kernel/config/config.h>
#include <kernel/elf/elf.h>
#include <kernel/error/error.h>
#include <kernel/fs/fs.h>
#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/pipe/pipe.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "os_loader"



#define MODULE_ORDER_FILE "/boot/module/module_order.config"



static void _load_modules_from_order_file(const char* order_file_path,_Bool early){
	vfs_node_t* file=vfs_lookup(NULL,order_file_path,0,0,0);
	if (!file){
		panic("Unable to locate module order file");
	}
	config_t* config=config_load(file);
	for (config_item_t* item=config->head;item;item=item->next){
		if (early!=(item->value&&streq(item->value->data,"early"))){
			continue;
		}
#if !KERNEL_COVERAGE_ENABLED
		if (item->value&&streq(item->value->data,"coverage")){
			continue;
		}
#endif
		module_load(item->key->data);
	}
	config_dealloc(config);
}



static _Bool _init(module_t* module){
	LOG("Loading early modules...");
	_load_modules_from_order_file(MODULE_ORDER_FILE,1);
	LOG("Searching for boot filesystem...");
	filesystem_t* boot_fs=NULL;
	HANDLE_FOREACH(fs_handle_type){
		handle_acquire(handle);
		filesystem_t* fs=handle->object;
		for (u8 i=0;i<16;i++){
			if (fs->uuid[i]!=kernel_data.boot_fs_uuid[i]){
				goto _check_next_fs;
			}
		}
		boot_fs=fs;
		break;
_check_next_fs:
		handle_release(handle);
	}
	if (!boot_fs){
		panic("Unable to find boot filesystem");
	}
	vfs_mount(boot_fs,NULL);
	LOG("Loading modules...");
	_load_modules_from_order_file(MODULE_ORDER_FILE,0);
	LOG("Loading user shell...");
	if (IS_ERROR(elf_load("/bin/shell",0,NULL,NULL,0))){
		panic("Unable to load user shell");
	}
	LOG("Loading UI...");
	if (vfs_lookup(NULL,"/bin/ui",0,0,0)&&IS_ERROR(elf_load("/bin/ui",0,NULL,NULL,0))){
		panic("Unable to load UI");
	}
	return 0;
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	_init,
	_deinit,
	MODULE_FLAG_PREVENT_LOADS
);
