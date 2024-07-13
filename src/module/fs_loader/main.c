#include <kernel/config/config.h>
#include <kernel/fs/fs.h>
#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/string.h>
#include <kernel/util/util.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "fs_loader"



#define MODULE_ORDER_FILE "/etc/fs_list.config"



static bool _parse_uuid(const char* uuid,u8* out){
	if (smm_length(uuid)!=36){
		return 0;
	}
	u32 j=0;
	for (u32 i=0;i<36;i++){
		if (0x842100&(1ull<<i)){
			if (uuid[i]!='-'){
				return 0;
			}
			continue;
		}
		if (!(j&1)){
			out[j>>1]=0;
		}
		out[j>>1]<<=4;
		if ('0'<=uuid[i]&&uuid[i]<='9'){
			out[j>>1]|=uuid[i]-'0';
		}
		else if ('A'<=uuid[i]&&uuid[i]<='Z'){
			out[j>>1]|=uuid[i]-'A'+10;
		}
		else if ('a'<=uuid[i]&&uuid[i]<='z'){
			out[j>>1]|=uuid[i]-'a'+10;
		}
		else{
			return 0;
		}
		j++;
	}
	return 1;
}



static filesystem_t* _match_fs(const char* uuid,const char* guid,const char* type){
	u8 uuid_data[16];
	if (uuid&&str_equal(uuid,"boot")){
		mem_copy(uuid_data,kernel_get_boot_uuid(),16);
	}
	else if (uuid&&!_parse_uuid(uuid,uuid_data)){
		ERROR("Invalid UUID: '%s'",uuid);
		return NULL;
	}
	u8 guid_data[16];
	if (guid&&!_parse_uuid(guid,guid_data)){
		ERROR("Invalid GUID: '%s'",guid);
		return NULL;
	}
	HANDLE_FOREACH(fs_handle_type){
		filesystem_t* fs=KERNEL_CONTAINEROF(handle,filesystem_t,handle);
		if (uuid){
			for (u8 i=0;i<16;i++){
				if (fs->uuid[i]!=uuid_data[i]){
					goto _check_next_fs;
				}
			}
		}
		if (guid){
			if (!fs->partition){
				goto _check_next_fs;
			}
			for (u8 i=0;i<16;i++){
				if (fs->partition->uuid[i]!=guid_data[i]){
					goto _check_next_fs;
				}
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
	vfs_node_unref(file);
	for (config_tag_t* fs_tag=config_tag_iter_start(root_tag);fs_tag;fs_tag=config_tag_iter_next(root_tag,fs_tag)){
		if (fs_tag->type!=CONFIG_TAG_TYPE_ARRAY){
			continue;
		}
		const char* path=NULL;
		const char* uuid=NULL;
		const char* guid=NULL;
		const char* type=NULL;
		bool required=1;
		config_tag_t* tag=NULL;
		if (config_tag_find(fs_tag,"path",0,&tag)&&tag->type==CONFIG_TAG_TYPE_STRING){
			path=tag->string->data;
		}
		if (config_tag_find(fs_tag,"uuid",0,&tag)&&tag->type==CONFIG_TAG_TYPE_STRING){
			uuid=tag->string->data;
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
		filesystem_t* fs=_match_fs(uuid,guid,type);
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
