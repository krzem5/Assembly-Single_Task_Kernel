#include <kernel/aes/aes.h>
#include <kernel/hash/sha256.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/random/random.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "keyring_master_key"



static u8 _keyring_master_key_encrypted[64];
static u8 _keyring_master_key[32];
static u8 _keyring_master_key_present=0;



static void _xor_block(u8* dst,const u8* src){
	for (u32 i=0;i<16;i++){
		dst[i]^=src[i];
	}
}



KERNEL_EARLY_EARLY_INIT(){
	LOG("Loading master key...");
	for (u32 i=0;i<64;i++){
		_keyring_master_key_encrypted[i]=kernel_data.master_key[i];
		_keyring_master_key_present|=_keyring_master_key_encrypted[i];
		kernel_data.master_key[i]=0;
	}
	_keyring_master_key_present=!!_keyring_master_key_present;
}



void keyring_master_key_set_platform_key(u8* platform_key,u8* master_key){
	LOG("Decrypting master key...");
	hash_sha256_state_t sha256_state;
	hash_sha256_init(&sha256_state);
	hash_sha256_process_chunk(&sha256_state,platform_key,32);
	hash_sha256_finalize(&sha256_state);
	aes_state_t state;
	aes_init(platform_key,32,AES_FLAG_ENCRYPTION|AES_FLAG_DECRYPTION,&state);
	if (_keyring_master_key_present){
		for (u32 i=64;i;){
			i-=16;
			aes_decrypt_block(&state,_keyring_master_key_encrypted+i,_keyring_master_key_encrypted+i);
			_xor_block(_keyring_master_key_encrypted+i,(i?_keyring_master_key_encrypted+(i-16):sha256_state.result));
		}
		memcpy(_keyring_master_key,_keyring_master_key_encrypted+32,32);
	}
	else if (master_key){
		INFO("Generating master key (external RNG)...");
		memcpy(_keyring_master_key,master_key,32);
	}
	else{
		INFO("Generating master key (internal RNG)...");
		random_generate(_keyring_master_key,32);
	}
	random_generate(_keyring_master_key_encrypted,32);
	memcpy(_keyring_master_key_encrypted+32,_keyring_master_key,32);
	for (u32 i=0;i<64;i+=16){
		_xor_block(_keyring_master_key_encrypted+i,(i?_keyring_master_key_encrypted+(i-16):sha256_state.result));
		aes_encrypt_block(&state,_keyring_master_key_encrypted+i,_keyring_master_key_encrypted+i);
	}
	aes_deinit(&state);
	memset(platform_key,0,32);
	if (master_key){
		memset(master_key,0,32);
	}
	for (u32 i=0;i<64;i+=8){
		WARN("[%u] %p",i,*((u64*)(_keyring_master_key_encrypted+i)));
	}
}
