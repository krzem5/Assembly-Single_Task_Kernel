#include <kernel/config/config.h>
#include <kernel/format/format.h>
#include <kernel/keyring/keyring.h>
#include <kernel/log/log.h>
#include <kernel/memory/smm.h>
#include <kernel/module/module.h>
#include <kernel/notification/notification.h>
#include <kernel/types.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "keyringstore"



#define KEYRING_STORE_DIRECTORY "/keyringstore"



static vfs_node_t* _keyringstore_root_dir=NULL;



static vfs_node_t* _get_store_directory(void){
	vfs_node_t* parent;
	const char* child_name;
	vfs_node_t* out=vfs_lookup_for_creation(NULL,KEYRING_STORE_DIRECTORY,0,0,0,&parent,&child_name);
	if (!out){
		INFO("Generating keyringstore directory...");
		SMM_TEMPORARY_STRING child_name_string=smm_alloc(child_name,0);
		out=vfs_node_create(NULL,parent,child_name_string,VFS_NODE_TYPE_DIRECTORY|VFS_NODE_FLAG_CREATE);
		if (!out){
			return NULL;
		}
	}
	out->uid=0;
	out->gid=0;
	out->flags|=(0500<<VFS_NODE_PERMISSION_SHIFT)|VFS_NODE_FLAG_DIRTY;
	vfs_node_flush(out);
	return out;
}



static config_tag_t* _generate_keyring_config(keyring_t* keyring){
	config_tag_t* root_tag=config_tag_create(CONFIG_TAG_TYPE_ARRAY,"");
	config_tag_t* name_tag=config_tag_create(CONFIG_TAG_TYPE_STRING,"name");
	config_tag_attach(root_tag,name_tag);
	smm_dealloc(name_tag->string);
	name_tag->string=smm_duplicate(keyring->name);
	config_tag_t* keys_tag=config_tag_create(CONFIG_TAG_TYPE_ARRAY,"keys");
	config_tag_attach(root_tag,keys_tag);
	for (keyring_key_t* key=keyring->head;key;key=key->next){
		config_tag_t* key_tag=config_tag_create(CONFIG_TAG_TYPE_ARRAY,"key");
		name_tag=config_tag_create(CONFIG_TAG_TYPE_STRING,"name");
		config_tag_attach(key_tag,name_tag);
		smm_dealloc(name_tag->string);
		name_tag->string=smm_duplicate(key->name);
		config_tag_attach(keys_tag,key_tag);
	}
	return root_tag;
}



static void _store_keyring(keyring_t* keyring){
	INFO("Storing keyring '%s'...",keyring->name->data);
	config_tag_t* root_tag=_generate_keyring_config(keyring);
	char buffer[32];
	format_string(buffer,32,"%lu.config",HANDLE_ID_GET_INDEX(keyring->handle.rb_node.key));
	vfs_node_t* parent;
	const char* child_name;
	vfs_node_t* node=vfs_lookup_for_creation(_keyringstore_root_dir,buffer,0,0,0,&parent,&child_name);
	if (!node){
		SMM_TEMPORARY_STRING child_name_string=smm_alloc(child_name,0);
		node=vfs_node_create(NULL,_keyringstore_root_dir,child_name_string,VFS_NODE_TYPE_FILE|VFS_NODE_FLAG_CREATE);
		if (!node){
			return;
		}
	}
	node->uid=0;
	node->gid=0;
	node->flags|=(0400<<VFS_NODE_PERMISSION_SHIFT)|VFS_NODE_FLAG_DIRTY;
	vfs_node_flush(node);
	config_save_to_file(root_tag,node,NULL);
	config_tag_delete(root_tag);
}



static void _keyring_update_callback(void* object,u32 type){
	keyring_t* keyring=object;
	if (type==NOTIFICATION_TYPE_KEYRING_UPDATE){
		_store_keyring(keyring);
		return;
	}
}



MODULE_PREINIT(){
	LOG("Initializing keyringstore...");
	_keyringstore_root_dir=_get_store_directory();
	if (!_keyringstore_root_dir){
		ERROR("Unable to create keyringstore directory");
		return 0;
	}
	INFO("Loading keyrings...");
	keyring_register_notification_listener(_keyring_update_callback);
	return 1;
}
