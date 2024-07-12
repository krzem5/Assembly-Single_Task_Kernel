#include <kernel/fs/fs.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <kernel/util/string.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "fat32"



typedef struct _FAT16_FILESYSTEM{
	char tmp;
} fat32_filesystem_t;



typedef struct _FAT16_VFS_NODE{
	vfs_node_t node;
} fat32_vfs_node_t;




static omm_allocator_t* KERNEL_INIT_WRITE _fat32_vfs_node_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _fat32_filesystem_allocator=NULL;
static filesystem_descriptor_t* KERNEL_INIT_WRITE _fat32_filesystem_descriptor=NULL;



static const vfs_functions_t _fat32_functions={
	NULL, // _fat32_create
	NULL, // _fat32_delete
	NULL, // _fat32_lookup
	NULL, // _fat32_iterate
	NULL, // _fat32_link
	NULL, // _fat32_unlink
	NULL, // _fat32_read
	NULL, // _fat32_write
	NULL, // _fat32_resize
	NULL // _fat32_flush
};



static void _fat32_fs_deinit(filesystem_t* fs){
	omm_dealloc(_fat32_filesystem_allocator,fs->extra_data);
	panic("_fat32_deinit_callback");
}



static filesystem_t* _fat32_fs_load(partition_t* partition){
	drive_t* drive=partition->drive;
	if (drive->block_size!=512){
		return NULL;
	}
	u8 buffer[512];
	if (drive_read(drive,partition->start_lba,buffer,1)!=1||(buffer[66]!=0x28&&buffer[66]!=0x29)||buffer[510]!=0x55||buffer[511]!=0xaa){
		return NULL;
	}
	if (buffer[66]==0x28){
		ERROR("Unimplemented FAT32 alternate signature");
		return NULL;
	}
	if (((buffer[18]<<8)|buffer[17])||((buffer[23]<<8)|buffer[22])||buffer[21]!=0xf8){
		ERROR("Unsupported FAT32 configuration");
		return NULL;
	}
	if (buffer[42]||buffer[43]){
		ERROR("Unsupported FAT32 version");
		return NULL;
	}
	u32 sector_size=(buffer[12]<<8)|buffer[11];
	if (sector_size!=512){
		ERROR("Unsupported FAT32 sector size: %u",sector_size);
		return NULL;
	}
	char oem_name[9];
	str_copy_from_padded((const char*)buffer+3,oem_name,8);
	INFO("Found FAT32 filesystem '%s'",oem_name);
	u32 sectors_per_cluster=buffer[13];
	u32 reserved_sectors=*((const u16*)(buffer+14));
	u32 fat_count=buffer[16];
	u64 sector_count=*((const u16*)(buffer+19));
	if (!sector_count){
		sector_count=*((const u32*)(buffer+32));
	}
	u32 sectors_per_fat=*((const u32*)(buffer+36));
	u32 root_directory_cluster=*((const u32*)(buffer+44));
	if (!sector_count){
		sector_count=*((const u64*)(buffer+82));
	}
	WARN("oem_name=%s",oem_name);
	WARN("sectors_per_cluster=%u",sectors_per_cluster);
	WARN("reserved_sectors=%u",reserved_sectors);
	WARN("fat_count=%u",fat_count);
	WARN("sector_count=%u",sector_count);
	WARN("sectors_per_fat=%u",sectors_per_fat);
	WARN("root_directory_cluster=%u",root_directory_cluster);
	// if (drive_read(drive,partition->start_lba+reserved_sectors+1,buffer,1)!=1){
	// 	return NULL;
	// }
	// for (u32 i=0;i<512;i+=8){
	// 	WARN("[%u]: %X %X %X %X %X %X %X %X | %c %c %c %c %c %c %c %c",i,buffer[i],buffer[i+1],buffer[i+2],buffer[i+3],buffer[i+4],buffer[i+5],buffer[i+6],buffer[i+7],buffer[i],buffer[i+1],buffer[i+2],buffer[i+3],buffer[i+4],buffer[i+5],buffer[i+6],buffer[i+7]);
	// }
	// if (drive_read(drive,partition->start_lba+reserved_sectors+sectors_per_fat*fat_count+0x71-root_directory_cluster,buffer,1)!=1){
	// 	return NULL;
	// }
	// for (u32 i=0;i<512;i+=8){
	// 	WARN("[%u]: %X %X %X %X %X %X %X %X | %c %c %c %c %c %c %c %c",i,buffer[i],buffer[i+1],buffer[i+2],buffer[i+3],buffer[i+4],buffer[i+5],buffer[i+6],buffer[i+7],buffer[i],buffer[i+1],buffer[i+2],buffer[i+3],buffer[i+4],buffer[i+5],buffer[i+6],buffer[i+7]);
	// }
	return NULL;
	(void)_fat32_functions;
}



static const filesystem_descriptor_config_t _fat32_filesystem_descriptor_config={
	"fat32",
	_fat32_fs_deinit,
	_fat32_fs_load,
	NULL,
	NULL
};



MODULE_INIT(){
	_fat32_vfs_node_allocator=omm_init("fat32.node",sizeof(fat32_vfs_node_t),8,4);
	rwlock_init(&(_fat32_vfs_node_allocator->lock));
	_fat32_filesystem_allocator=omm_init("fat32.filesystem",sizeof(fat32_filesystem_t),8,1);
	rwlock_init(&(_fat32_filesystem_allocator->lock));
}



MODULE_POSTINIT(){
	_fat32_filesystem_descriptor=fs_register_descriptor(&_fat32_filesystem_descriptor_config);
}
