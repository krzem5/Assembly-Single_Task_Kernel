#include <kernel/config/config.h>
#include <kernel/fs/fs.h>
#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "fs_loader"



#define MODULE_ORDER_FILE "/boot/module/fs_list.config"



static filesystem_t* _match_fs(const char* guid,const char* type){
	HANDLE_FOREACH(fs_handle_type){
		handle_acquire(handle);
		filesystem_t* fs=handle->object;
		if (guid){
			if (streq(guid,"boot")){
				for (u8 i=0;i<16;i++){
					if (fs->uuid[i]!=kernel_data.boot_fs_uuid[i]){
						goto _check_next_fs;
					}
				}
			}
			else{
				ERROR("Unimplemented: _match_fs.guid");
			}
		}
		if (type&&!streq(fs->descriptor->config->name,type)){
			goto _check_next_fs;
		}
		return fs;
_check_next_fs:
		handle_release(handle);
	}
	return NULL;
}



static _Bool _init(module_t* module){
	LOG("Mounting filesystems...");
	vfs_node_t* file=vfs_lookup(NULL,MODULE_ORDER_FILE,0,0,0);
	if (!file){
		panic("Unable to locate filesystem list file");
	}
	config_t* config=config_load(file);
	const char* path=NULL;
	const char* guid=NULL;
	const char* type=NULL;
	_Bool required=1;
	for (config_item_t* item=config->head;item;item=item->next){
		if (streq(item->key->data,"path")){
			path=item->value->data;
		}
		else if (streq(item->key->data,"guid")){
			guid=item->value->data;
		}
		else if (streq(item->key->data,"type")){
			type=item->value->data;
		}
		else if (streq(item->key->data,"not-required")){
			required=0;
		}
		if (!item->next||streq(item->next->key->data,"path")){
			if (!path){
				ERROR("Malformated file");
			}
			else{
				filesystem_t* fs=_match_fs(guid,type);
				if (!fs&&required){
					ERROR("Filesystem not found");
				}
				else if (fs){
					vfs_mount(fs,(streq(path,"/")?NULL:path),0);
				}
			}
			path=NULL;
			guid=NULL;
			type=NULL;
			required=1;
		}
	}
	config_dealloc(config);
	return 0;
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	_init,
	_deinit,
	0
);
