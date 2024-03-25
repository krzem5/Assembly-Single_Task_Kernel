#include <kernel/aes/aes.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "keyring_master_key"



static u8 _keyring_master_key[32];
static u8 _keyring_master_key_present=0;



KERNEL_EARLY_EARLY_INIT(){
	LOG("Clearing platform key...");
	for (u32 i=0;i<32;i++){
		_keyring_master_key[i]=kernel_data.master_key[i];
		_keyring_master_key_present|=_keyring_master_key[i];
		kernel_data.master_key[i]=0;
	}
	_keyring_master_key_present=!!_keyring_master_key_present;
}



void keyring_master_key_set_platform_key(u8* key){
	// aes_state_t state;
	// aes_init(key,32,AES_FLAG_ENCRYPTION|AES_FLAG_DECRYPTION,&state);
	// aes_decrypt_block(&state,_keyring_master_key,_keyring_master_key);
	// aes_decrypt_block(&state,_keyring_master_key+16,_keyring_master_key+16);
	// aes_deinit(&state);
	// memset(key,0,32);
	for (u32 i=0;i<32;i++){
		WARN("%X",_keyring_master_key[i]);
	}
	// panic("A");
}
