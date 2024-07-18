#include <kernel/config/config.h>
#include <kernel/format/format.h>
#include <kernel/keyring/keyring.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/smm.h>
#include <kernel/module/module.h>
#include <kernel/mp/thread.h>
#include <kernel/notification/notification.h>
#include <kernel/rsa/rsa.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "keyringstore"



#define KEYRING_STORE_DIRECTORY "/etc/keyringstore"



static vfs_node_t* KERNEL_INIT_WRITE _keyringstore_root_dir=NULL;



static vfs_node_t* KERNEL_AWAITS_EARLY _get_store_directory(void){
	vfs_node_t* parent;
	const char* child_name;
	vfs_node_t* out=vfs_lookup_for_creation(NULL,KEYRING_STORE_DIRECTORY,0,0,0,&parent,&child_name);
	if (!out){
		INFO("Generating keyringstore directory...");
		SMM_TEMPORARY_STRING child_name_string=smm_alloc(child_name,0);
		out=vfs_node_create(NULL,parent,child_name_string,VFS_NODE_TYPE_DIRECTORY|VFS_NODE_FLAG_CREATE);
		vfs_node_unref(parent);
		if (!out){
			return NULL;
		}
	}
	out->uid=0;
	out->gid=0;
	out->flags&=~VFS_NODE_PERMISSION_MASK;
	out->flags|=(0000<<VFS_NODE_PERMISSION_SHIFT)|VFS_NODE_FLAG_DIRTY;
	vfs_node_flush(out);
	return out;
}



static void KERNEL_AWAITS_EARLY _load_keyring(config_tag_t* root_tag){
	config_tag_t* name_tag;
	if (!config_tag_find(root_tag,"name",0,&name_tag)||name_tag->type!=CONFIG_TAG_TYPE_STRING){
		return;
	}
	INFO("Loading keyring '%s'...",name_tag->string->data);
	keyring_t* keyring=keyring_create(name_tag->string->data);
	config_tag_t* keys_tag;
	if (!config_tag_find(root_tag,"keys",0,&keys_tag)||keys_tag->type!=CONFIG_TAG_TYPE_ARRAY){
		return;
	}
	config_tag_t* key_tag;
	for (u64 pointer=config_tag_find(keys_tag,"key",0,&key_tag);pointer;pointer=config_tag_find(keys_tag,"key",pointer,&key_tag)){
		if (key_tag->type!=CONFIG_TAG_TYPE_ARRAY){
			continue;
		}
		if (!config_tag_find(key_tag,"name",0,&name_tag)||name_tag->type!=CONFIG_TAG_TYPE_STRING){
			continue;
		}
		config_tag_t* type_tag;
		if (!config_tag_find(key_tag,"type",0,&type_tag)||type_tag->type!=CONFIG_TAG_TYPE_INT){
			continue;
		}
		config_tag_t* flags_tag;
		if (!config_tag_find(key_tag,"flags",0,&flags_tag)||flags_tag->type!=CONFIG_TAG_TYPE_INT){
			continue;
		}
		keyring_key_t* key=keyring_key_create(keyring,name_tag->string->data);
		if (!key){
			WARN("Skipping key '%s'...",name_tag->string->data);
			continue;
		}
		key->type=type_tag->int_;
		key->flags=flags_tag->int_;
		if (key->type==KEYRING_KEY_TYPE_NONE){
			continue;
		}
		if (key->type==KEYRING_KEY_TYPE_RAW){
			config_tag_t* raw_tag;
			if (!config_tag_find(key_tag,"raw",0,&raw_tag)||raw_tag->type!=CONFIG_TAG_TYPE_STRING){
				key->type=KEYRING_KEY_TYPE_NONE;
				continue;
			}
			key->data.raw.payload=amm_alloc(raw_tag->string->length);
			key->data.raw.payload_length=raw_tag->string->length;
			mem_copy(key->data.raw.payload,raw_tag->string->data,raw_tag->string->length);
			continue;
		}
		ERROR("Load key from config: RSA key");
		for (;;);
	}
}



static void KERNEL_AWAITS_EARLY _load_keyrings(void){
	u64 pointer=0;
	string_t* name;
	while (1){
		pointer=vfs_node_iterate(_keyringstore_root_dir,pointer,&name);
		if (!pointer){
			return;
		}
		vfs_node_t* node=vfs_lookup(_keyringstore_root_dir,name->data,0,0,0);
		smm_dealloc(name);
		if (!node){
			continue;
		}
		config_tag_t* root_tag=config_load_from_file(node,CONFIG_PASSWORD_MASTER_KEY);
		vfs_node_unref(node);
		if (!root_tag){
			continue;
		}
		_load_keyring(root_tag);
		config_tag_delete(root_tag);
	}
}



static void _generate_rsa_number_config(config_tag_t* root_tag,const rsa_number_t* number,const char* name){
	if (!number){
		return;
	}
	config_tag_t* tag=config_tag_create(CONFIG_TAG_TYPE_STRING,name);
	config_tag_attach(root_tag,tag);
	smm_dealloc(tag->string);
	tag->string=smm_alloc((void*)(number->data),number->length*sizeof(u32));
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
		if (key->flags&KEYRING_KEY_FLAG_VIRTUAL){
			continue;
		}
		config_tag_t* key_tag=config_tag_create(CONFIG_TAG_TYPE_ARRAY,"key");
		name_tag=config_tag_create(CONFIG_TAG_TYPE_STRING,"name");
		config_tag_attach(key_tag,name_tag);
		smm_dealloc(name_tag->string);
		name_tag->string=smm_duplicate(key->name);
		config_tag_t* type_tag=config_tag_create(CONFIG_TAG_TYPE_INT,"type");
		config_tag_attach(key_tag,type_tag);
		type_tag->int_=key->type;
		config_tag_t* flags_tag=config_tag_create(CONFIG_TAG_TYPE_INT,"flags");
		config_tag_attach(key_tag,flags_tag);
		flags_tag->int_=key->flags;
		if (key->type==KEYRING_KEY_TYPE_NONE){
			config_tag_attach(key_tag,config_tag_create(CONFIG_TAG_TYPE_NONE,"none"));
		}
		else if (key->type==KEYRING_KEY_TYPE_RAW){
			config_tag_t* raw_tag=config_tag_create(CONFIG_TAG_TYPE_STRING,"raw");
			config_tag_attach(key_tag,raw_tag);
			smm_dealloc(raw_tag->string);
			raw_tag->string=smm_alloc(key->data.raw.payload,key->data.raw.payload_length);
		}
		else if (key->type==KEYRING_KEY_TYPE_RSA){
			config_tag_t* rsa_tag=config_tag_create(CONFIG_TAG_TYPE_ARRAY,"rsa");
			config_tag_attach(key_tag,rsa_tag);
			_generate_rsa_number_config(rsa_tag,key->data.rsa.state.modulus,"modulus");
			_generate_rsa_number_config(rsa_tag,key->data.rsa.state.private_key,"private");
			_generate_rsa_number_config(rsa_tag,key->data.rsa.state.public_key,"public");
		}
		else{
			ERROR("_generate_keyring_config: unknown key type '%u'",key->type);
		}
		config_tag_attach(keys_tag,key_tag);
	}
	return root_tag;
}



static KERNEL_AWAITS void _store_keyring(keyring_t* keyring){
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
		vfs_node_unref(parent);
		if (!node){
			return;
		}
	}
	node->uid=0;
	node->gid=0;
	node->flags&=~VFS_NODE_PERMISSION_MASK;
	node->flags|=(0000<<VFS_NODE_PERMISSION_SHIFT)|VFS_NODE_FLAG_DIRTY;
	vfs_node_flush(node);
	config_save_to_file(root_tag,node,CONFIG_PASSWORD_MASTER_KEY,0);
	vfs_node_unref(node);
	config_tag_delete(root_tag);
}



static KERNEL_AWAITS void _keyring_update_thread(void){
	notification_consumer_t* consumer=notification_consumer_create(keyring_notification_dispatcher);
	HANDLE_FOREACH(keyring_handle_type){
		_store_keyring(KERNEL_CONTAINEROF(handle,keyring_t,handle));
	}
	while (1){
		notification_t notification;
		if (!notification_consumer_get(consumer,1,&notification)||notification.type!=KEYRING_UPDATE_NOTIFICATION||notification.length!=sizeof(keyring_update_notification_data_t)){
			continue;
		}
		const keyring_update_notification_data_t* data=notification.data;
		handle_t* handle=handle_lookup_and_acquire(data->keyring_handle,keyring_handle_type);
		if (!handle){
			continue;
		}
		_store_keyring(KERNEL_CONTAINEROF(handle,keyring_t,handle));
		handle_release(handle);
	}
}



MODULE_INIT(){
	LOG("Initializing keyringstore...");
	_keyringstore_root_dir=_get_store_directory();
	if (!_keyringstore_root_dir){
		ERROR("Unable to create keyringstore directory");
		module_unload(module_self);
		return;
	}
	INFO("Loading keyrings...");
	_load_keyrings();
	thread_create_kernel_thread(NULL,"keyringstore.update",_keyring_update_thread,0);
}
