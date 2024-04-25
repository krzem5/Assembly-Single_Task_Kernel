#include <common/aes/aes.h>
#include <common/update/update.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/random/random.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "keyring_master_key"



static u8 _keyring_master_key_encrypted[64];
static u8 _keyring_platform_key_encrypted[64];

u8 keyring_master_key[32];



KERNEL_EARLY_EARLY_INIT(){
	LOG("Loading master key...");
	for (u32 i=0;i<64;i++){
		_keyring_master_key_encrypted[i]=kernel_data.master_key[i];
		kernel_data.master_key[i]=0;
	}
}



KERNEL_PUBLIC void keyring_master_key_get_encrypted(u8* buffer,u32 buffer_size){
	if (buffer_size!=64){
		panic("keyring_master_key_get_encrypted: invalid buffer size");
	}
	mem_copy(buffer,_keyring_master_key_encrypted,64);
}



void keyring_master_key_set_platform_key(u8* platform_key,u8* master_key){
	LOG("Decrypting master key...");
	bool master_key_present=0;
	for (u32 i=0;i<64;i++){
		master_key_present|=_keyring_master_key_encrypted[i];
	}
	u8 iv[16];
	if (master_key_present){
		u8 buffer[64];
		random_generate(iv,sizeof(iv));
		aes_cbc_process(platform_key,32,iv,sizeof(iv),AES_FLAG_DECRYPTION,_keyring_master_key_encrypted,64,buffer);
		mem_copy(keyring_master_key,buffer+32,32);
		mem_fill(buffer,sizeof(buffer),0);
	}
	else if (master_key){
		INFO("Generating master key (external RNG)...");
		mem_copy(keyring_master_key,master_key,32);
	}
	else{
		INFO("Generating master key (internal RNG)...");
		random_generate(keyring_master_key,32);
	}
#ifndef KERNEL_RELEASE
	mem_copy(keyring_master_key,"non-release-constant-master-key\x7f",32);
#endif
	random_generate(iv,sizeof(iv));
	random_generate(_keyring_master_key_encrypted,32);
	mem_copy(_keyring_master_key_encrypted+32,keyring_master_key,32);
	aes_cbc_process(platform_key,32,iv,sizeof(iv),AES_FLAG_ENCRYPTION,_keyring_master_key_encrypted,64,_keyring_master_key_encrypted);
	random_generate(iv,sizeof(iv));
	random_generate(_keyring_platform_key_encrypted,32);
	mem_copy(_keyring_platform_key_encrypted+32,platform_key,32);
	aes_cbc_process(keyring_master_key,32,iv,sizeof(iv),AES_FLAG_ENCRYPTION,_keyring_platform_key_encrypted,64,_keyring_platform_key_encrypted);
	mem_fill(platform_key,32,0);
	if (master_key){
		mem_fill(master_key,32,0);
	}
}



KERNEL_PUBLIC void keyring_master_key_generate_update_ticket(u8* new_kernel_hash,u8* new_initramfs_hash){
	LOG("Generating update ticket...");
	vfs_node_t* parent;
	const char* child_name;
	vfs_node_t* node=vfs_lookup_for_creation(NULL,"/boot/update_ticket",0,0,0,&parent,&child_name);
	if (!node){
		SMM_TEMPORARY_STRING child_name_string=smm_alloc(child_name,0);
		node=vfs_node_create(NULL,parent,child_name_string,VFS_NODE_TYPE_FILE|VFS_NODE_FLAG_CREATE);
		if (!node){
			return;
		}
	}
	node->uid=0;
	node->gid=0;
	node->flags&=~VFS_NODE_PERMISSION_MASK;
	node->flags|=(0000<<VFS_NODE_PERMISSION_SHIFT)|VFS_NODE_FLAG_DIRTY;
	vfs_node_flush(node);
	if (vfs_node_resize(node,sizeof(update_ticket_t),0)!=sizeof(update_ticket_t)){
		return;
	}
	u8 buffer[64];
	mem_copy(buffer,new_kernel_hash,32);
	mem_copy(buffer+32,new_initramfs_hash,32);
	mem_fill(new_kernel_hash,32,0);
	mem_fill(new_initramfs_hash,32,0);
	update_ticket_t update_ticket;
	mem_fill(buffer,64,0);
	mem_fill(&update_ticket,sizeof(update_ticket_t),0);
	if (vfs_node_write(node,0,&update_ticket,sizeof(update_ticket_t),0)!=sizeof(update_ticket_t)){
		return;
	}
	// panic("keyring_master_key_generate_update_ticket");
}
