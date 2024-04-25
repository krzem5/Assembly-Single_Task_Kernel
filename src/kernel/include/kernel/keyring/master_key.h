#ifndef _KERNEL_KEYRING_MASTER_KEY_H_
#define _KERNEL_KEYRING_MASTER_KEY_H_ 1
#include <kernel/types.h>



extern u8 keyring_master_key[32];



void keyring_master_key_get_encrypted(u8* buffer,u32 buffer_size);



void keyring_master_key_set_platform_key(u8* platform_key,u8* master_key);



void keyring_master_key_generate_update_ticket(u8* new_kernel_hash,u8* new_initramfs_hash);



#endif
