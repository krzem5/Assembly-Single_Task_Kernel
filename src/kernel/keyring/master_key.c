#include <kernel/log/log.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "keyring_master_key"



static volatile u8 _keyring_master_key_platform_key[32];



KERNEL_EARLY_EARLY_INIT(){
	LOG("Clearing platform key...");
	for (u32 i=0;i<32;i++){
		_keyring_master_key_platform_key[i]=0;
	}
}



void keyring_master_key_set_platform_key(u8* key){
	for (u32 i=0;i<32;i++){
		_keyring_master_key_platform_key[i]=key[i];
		key[i]=0;
	}
}
