#include <kernel/aes/aes.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/random/random.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/util.h>
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
	mem_fill(iv,sizeof(iv),0);
	mem_fill(platform_key,32,0);
	if (master_key){
		mem_fill(master_key,32,0);
	}
}
