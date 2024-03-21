#include <kernel/hash/sha256.h>
#include <kernel/initramfs/drive.h>
#include <kernel/initramfs/fs.h>
#include <kernel/initramfs/partition.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/vmm.h>
#include <kernel/tpm/tpm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "initramfs"



static _Bool _initramfs_is_loaded=0;



KERNEL_EARLY_INIT(){
	LOG("Checking initramfs signature...");
	hash_sha256_state_t state;
	hash_sha256_init(&state);
	hash_sha256_process_chunk(&state,(const void*)(kernel_data.initramfs_address+VMM_HIGHER_HALF_ADDRESS_OFFSET),kernel_data.initramfs_size);
	hash_sha256_finalize(&state);
	u32 buffer[8];
	for (u32 i=0;i<8;i++){
		buffer[i]=__builtin_bswap32(state.result[i]);
	}
	tpm_register_signature(TPM_SIGNATURE_TYPE_INITRAMFS_SHA256,buffer);
}



KERNEL_INIT(){
	LOG("Loading initramfs...");
	initramfs_fs_init();
	initramfs_partition_init();
	initramfs_drive_init();
	_initramfs_is_loaded=1;
}



KERNEL_PUBLIC void initramfs_unload(void){
	LOG("Unloading initramfs...");
	if (!_initramfs_is_loaded){
		return;
	}
	_initramfs_is_loaded=0;
	initramfs_drive_deinit();
	initramfs_partition_deinit();
	initramfs_fs_deinit();
}
