#include <kernel/aes/aes.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/random/random.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "keyring_master_key"



static u8 _keyring_master_key_encrypted[64];

u8 keyring_master_key[32];



static void _xor_block(u8* dst,const u8* src){
	for (u32 i=0;i<16;i++){
		dst[i]^=src[i];
	}
}



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
	_Bool master_key_present=0;
	for (u32 i=0;i<64;i++){
		master_key_present|=_keyring_master_key_encrypted[i];
	}
	aes_state_t state;
	aes_init(platform_key,32,AES_FLAG_ENCRYPTION|(master_key_present?AES_FLAG_DECRYPTION:0),&state);
	if (master_key_present){
		for (u32 i=48;i>=32;i-=16){
			aes_decrypt_block(&state,_keyring_master_key_encrypted+i,_keyring_master_key_encrypted+i);
			_xor_block(_keyring_master_key_encrypted+i,_keyring_master_key_encrypted+(i-16));
		}
		mem_copy(keyring_master_key,_keyring_master_key_encrypted+32,32);
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
	random_generate(_keyring_master_key_encrypted,32);
	mem_copy(_keyring_master_key_encrypted+32,keyring_master_key,32);
	for (u32 i=0;i<64;i+=16){
		if (i){
			_xor_block(_keyring_master_key_encrypted+i,_keyring_master_key_encrypted+(i-16));
		}
		aes_encrypt_block(&state,_keyring_master_key_encrypted+i,_keyring_master_key_encrypted+i);
	}
	aes_deinit(&state);
	mem_fill(platform_key,32,0);
	if (master_key){
		mem_fill(master_key,32,0);
	}
}
