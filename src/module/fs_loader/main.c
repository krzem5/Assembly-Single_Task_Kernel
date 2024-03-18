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
					if (fs->guid[i]!=kernel_data.boot_fs_guid[i]){
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
	config_tag_t* root_tag=config_load_from_file(file,NULL);
	if (!root_tag){
		panic("Unable to parse filesystem list file");
	}
	if (root_tag->type!=CONFIG_TAG_TYPE_ARRAY){
		panic("Invalid tag type");
	}
	for (u32 i=0;i<root_tag->array->length;i++){
		config_tag_t* tag=root_tag->array->data[i];
		if (tag->type!=CONFIG_TAG_TYPE_ARRAY){
			panic("Invalid tag type");
		}
		const char* path=NULL;
		const char* guid=NULL;
		const char* type=NULL;
		_Bool required=1;
		for (u32 j=0;j<tag->array->length;j++){
			config_tag_t* subtag=tag->array->data[j];
			if (streq(subtag->name->data,"path")){
				if (subtag->type!=CONFIG_TAG_TYPE_STRING){
					panic("Invalid tag type");
				}
				path=subtag->string->data;
			}
			else if (streq(subtag->name->data,"guid")){
				if (subtag->type!=CONFIG_TAG_TYPE_STRING){
					panic("Invalid tag type");
				}
				guid=subtag->string->data;
			}
			else if (streq(subtag->name->data,"type")){
				if (subtag->type!=CONFIG_TAG_TYPE_STRING){
					panic("Invalid tag type");
				}
				type=subtag->string->data;
			}
			else if (streq(subtag->name->data,"not-required")){
				required=0;
			}
		}
		if (!path){
			panic("'path' tag missing");
		}
		filesystem_t* fs=_match_fs(guid,type);
		if (!fs&&required){
			ERROR("Filesystem not found");
		}
		else if (fs){
			vfs_mount(fs,(streq(path,"/")?NULL:path),0);
		}
	}
	config_tag_delete(root_tag);
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
