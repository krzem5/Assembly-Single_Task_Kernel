#include <kernel/config/config.h>
#include <kernel/elf/elf.h>
#include <kernel/error/error.h>
#include <kernel/initramfs/initramfs.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <kernel/util/string.h>
#include <kernel/util/util.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "module_loader"



#define MODULE_ORDER_FILE "/boot/module/module_order.config"



static void _load_modules_from_order_file(_Bool early){
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
	if (IS_ERROR(elf_load("/bin/shell",0,NULL,0,NULL,0))){
		panic("Unable to load user shell");
	}
#endif
	return 0;
}



MODULE_DECLARE(MODULE_FLAG_PREVENT_LOADS);
