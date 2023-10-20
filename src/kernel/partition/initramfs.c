#include <kernel/drive/drive.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/vfs.h>



PARTITION_DECLARE_TYPE(INITRAMFS,{
	if (!streq(drive->type->name,"INITRAMFS")){
		return 0;
	}
	partition_t* partition=partition_create(drive,"INITRAMFS",0,drive->block_count);
	if (!partition->fs){
		return 0;
	}
	vfs_mount(partition->fs,NULL);
	return 1;
});
