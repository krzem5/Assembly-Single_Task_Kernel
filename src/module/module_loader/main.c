#include <kernel/config/config.h>
#include <kernel/elf/elf.h>
#include <kernel/error/error.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "module_loader"



#define MODULE_ORDER_FILE "/boot/module/module_order.config"



static void _load_modules_from_order_file(_Bool early){
	vfs_node_t* file=vfs_lookup(NULL,MODULE_ORDER_FILE,0,0,0);
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
#else
		if (item->value&&streq(item->value->data,"not-coverage")){
			continue;
		}
#endif
		module_load(item->key->data);
	}
	config_dealloc(config);
}



static _Bool _init(module_t* module){
	LOG("Loading early modules...");
	_load_modules_from_order_file(1);
	LOG("Loading modules...");
	_load_modules_from_order_file(0);
	LOG("Loading user shell...");
#if !KERNEL_COVERAGE_ENABLED
	if (IS_ERROR(elf_load("/bin/shell",0,NULL,0,NULL,0))){
		panic("Unable to load user shell");
	}
#endif
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
