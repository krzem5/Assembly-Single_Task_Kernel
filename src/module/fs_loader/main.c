#include <kernel/config/config.h>
#include <kernel/fs/fs.h>
#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <kernel/util/string.h>
#include <kernel/util/util.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "fs_loader"



#define MODULE_ORDER_FILE "/etc/fs_list.config"



static filesystem_t* _match_fs(const char* guid,const char* type){
	HANDLE_FOREACH(fs_handle_type){
		filesystem_t* fs=KERNEL_CONTAINEROF(handle,filesystem_t,handle);
		if (guid){
			if (str_equal(guid,"boot")){
				for (u8 i=0;i<16;i++){
					if (fs->guid[i]!=kernel_get_boot_guid()[i]){
						goto _check_next_fs;
					}
				}
			}
			else{
				ERROR("Unimplemented: _match_fs.guid");
			}
		}
		if (type&&!str_equal(fs->descriptor->config->name,type)){
			goto _check_next_fs;
		}
		return fs;
_check_next_fs:
	}
	return NULL;
}



MODULE_INIT(){
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
		config_tag_t* fs_tag=root_tag->array->data[i];
		if (fs_tag->type!=CONFIG_TAG_TYPE_ARRAY){
			panic("Invalid tag type");
		}
		const char* path=NULL;
		const char* guid=NULL;
		const char* type=NULL;
		bool required=1;
		config_tag_t* tag=NULL;
		if (config_tag_find(fs_tag,"path",0,&tag)&&tag->type==CONFIG_TAG_TYPE_STRING){
			path=tag->string->data;
		}
		if (config_tag_find(fs_tag,"guid",0,&tag)&&tag->type==CONFIG_TAG_TYPE_STRING){
			guid=tag->string->data;
		}
		if (config_tag_find(fs_tag,"type",0,&tag)&&tag->type==CONFIG_TAG_TYPE_STRING){
			type=tag->string->data;
		}
		if (config_tag_find(fs_tag,"not-required",0,&tag)){
			required=0;
		}
		if (!path){
			ERROR("'path' tag missing");
			continue;
		}
		filesystem_t* fs=_match_fs(guid,type);
		if (!fs&&required){
			ERROR("Filesystem not found");
		}
		else if (fs){
			vfs_mount(fs,(str_equal(path,"/")?NULL:path),0);
		}
		if (fs){
			handle_release(&(fs->handle));
		}
	}
	config_tag_delete(root_tag);
}



MODULE_DECLARE(0);
