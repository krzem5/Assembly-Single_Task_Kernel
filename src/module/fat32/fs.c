#include <kernel/fs/fs.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <kernel/util/string.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "fat32"



typedef struct _FAT32_FILESYSTEM{
	u32 sector_size;
	u32 sectors_per_cluster;
	u32 reserved_sectors;
	u32 sectors_per_fat;
	u64 sector_count;
	u32 cluster_offset;
	u64* fat_bitmap;
	u32* fat_data;
} fat32_filesystem_t;



typedef struct _FAT32_VFS_NODE{
	vfs_node_t node;
	u32 cluster;
	u32 size;
} fat32_vfs_node_t;




static pmm_counter_descriptor_t* KERNEL_INIT_WRITE _fat32_buffer_pmm_counter=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _fat32_vfs_node_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _fat32_filesystem_allocator=NULL;
static filesystem_descriptor_t* KERNEL_INIT_WRITE _fat32_filesystem_descriptor=NULL;



static u32 _lookup_next_cluster(fat32_filesystem_t* fs,u32 offset){
	ERROR("_lookup_next_cluster: %u",offset);
	return 0;
}



static u8* _lookup_dir_entry(fat32_filesystem_t* fs,partition_t* partition,u32 cluster,u8* buffer,u64* ptr,char* name){
	drive_t* drive=partition->drive;
	u64 pointer=(*ptr?*ptr:((u64)cluster)<<32);
	u32 buffer_offset=0;
	while (1){
		if ((pointer&0xffffffff)>=fs->sectors_per_cluster*fs->sector_size){
			pointer=((u64)_lookup_next_cluster(fs,pointer>>32))<<32;
			if (!pointer){
				return NULL;
			}
		}
		u32 pointer_offset=(pointer>>32)*fs->sectors_per_cluster+(pointer&0xffffffff)/fs->sector_size;
		if (pointer_offset!=buffer_offset){
			if (drive_read(drive,partition->start_lba+fs->cluster_offset+pointer_offset,buffer,1)!=1){
				return NULL;
			}
			buffer_offset=pointer_offset;
		}
		u32 offset=pointer&(fs->sector_size-1);
		if (buffer[offset]==0x05||buffer[offset]=='.'||buffer[offset]==0xe5||(buffer[offset+11]&0xc8)){
			pointer+=32;
			continue;
		}
		if (!buffer[offset]){
			return NULL;
		}
		char entry_name[13];
		str_copy_from_padded((const char*)(buffer+offset),entry_name,8);
		if (!(buffer[offset+11]&0x10)){
			char* end=entry_name+smm_length(entry_name);
			*end='.';
			str_copy_from_padded((const char*)(buffer+offset+8),end+1,3);
		}
		pointer+=32;
		if (!name[0]||str_equal(name,entry_name)){
			if (!name[0]){
				str_copy(entry_name,name,13);
			}
			*ptr=pointer;
			return buffer+((pointer-32)&(fs->sector_size-1));
		}
	}
	return NULL;
}



static vfs_node_t* _fat32_create(vfs_node_t* parent,const string_t* name,u32 flags){
	fat32_vfs_node_t* out=omm_alloc(_fat32_vfs_node_allocator);
	out->cluster=0;
	return (vfs_node_t*)out;
}



static void _fat32_delete(vfs_node_t* node){
	omm_dealloc(_fat32_vfs_node_allocator,node);
}



static vfs_node_t* _fat32_lookup(vfs_node_t* node,const string_t* name){
	if (name->length>12){
		return NULL;
	}
	fat32_vfs_node_t* fat32_node=(fat32_vfs_node_t*)node;
	u8 buffer[512];
	u64 pointer=0;
	const u8* entry=_lookup_dir_entry(node->fs->extra_data,node->fs->partition,fat32_node->cluster,buffer,&pointer,(char*)(name->data));
	if (!entry){
		return 0;
	}
	fat32_vfs_node_t* out=(fat32_vfs_node_t*)vfs_node_create(node->fs,NULL,name,0);
	out->node.flags|=(0755<<VFS_NODE_PERMISSION_SHIFT);
	if (entry[11]&0x10){
		out->node.flags|=VFS_NODE_TYPE_DIRECTORY;
	}
	else{
		out->node.flags|=VFS_NODE_TYPE_FILE;
	}
	out->cluster=(*((const u16*)(entry+26)))|((*((const u16*)(entry+20)))<<16);
	out->size=*((const u32*)(entry+28));
	return (vfs_node_t*)out;
}



static u64 _fat32_iterate(vfs_node_t* node,u64 pointer,string_t** out){
	fat32_vfs_node_t* fat32_node=(fat32_vfs_node_t*)node;
	u8 buffer[512];
	char name[13]={0};
	if (!_lookup_dir_entry(node->fs->extra_data,node->fs->partition,fat32_node->cluster,buffer,&pointer,name)){
		return 0;
	}
	*out=smm_alloc(name,0);
	return pointer;
}



static const vfs_functions_t _fat32_functions={
	_fat32_create,
	_fat32_delete,
	_fat32_lookup,
	_fat32_iterate,
	NULL,
	NULL,
	NULL, // _fat32_read
	NULL,
	NULL, // _fat32_resize
	NULL
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
	fat32_filesystem_t* extra_data=omm_alloc(_fat32_filesystem_allocator);
	extra_data->sector_size=sector_size;
	extra_data->sectors_per_cluster=buffer[13];
	extra_data->reserved_sectors=*((const u16*)(buffer+14));
	extra_data->sectors_per_fat=*((const u32*)(buffer+36));
	extra_data->sector_count=*((const u16*)(buffer+19));
	if (!extra_data->sector_count){
		extra_data->sector_count=*((const u32*)(buffer+32));
	}
	if (!extra_data->sector_count){
		extra_data->sector_count=*((const u64*)(buffer+82));
	}
	extra_data->cluster_offset=extra_data->reserved_sectors+extra_data->sectors_per_fat*buffer[16]-2;
	extra_data->fat_bitmap=amm_alloc((extra_data->sectors_per_fat*extra_data->sector_size/sizeof(u32)+63)>>6);
	extra_data->fat_data=(void*)(pmm_alloc(pmm_align_up_address(extra_data->sectors_per_fat*extra_data->sector_size)>>PAGE_SIZE_SHIFT,_fat32_buffer_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	u32 root_directory_cluster=*((const u32*)(buffer+44));
	filesystem_t* out=fs_create(_fat32_filesystem_descriptor);
	out->functions=&_fat32_functions;
	out->partition=partition;
	out->extra_data=extra_data;
	SMM_TEMPORARY_STRING root_name=smm_alloc("",0);
	out->root=vfs_node_create(out,NULL,root_name,0);
	out->root->flags|=VFS_NODE_TYPE_DIRECTORY|VFS_NODE_FLAG_PERMANENT|(0755<<VFS_NODE_PERMISSION_SHIFT);
	((fat32_vfs_node_t*)(out->root))->cluster=root_directory_cluster;
	return out;
}



static const filesystem_descriptor_config_t _fat32_filesystem_descriptor_config={
	"fat32",
	_fat32_fs_deinit,
	_fat32_fs_load,
	NULL,
	NULL
};



MODULE_INIT(){
	_fat32_buffer_pmm_counter=pmm_alloc_counter("fat32.buffer");
	_fat32_vfs_node_allocator=omm_init("fat32.node",sizeof(fat32_vfs_node_t),8,4);
	rwlock_init(&(_fat32_vfs_node_allocator->lock));
	_fat32_filesystem_allocator=omm_init("fat32.filesystem",sizeof(fat32_filesystem_t),8,1);
	rwlock_init(&(_fat32_filesystem_allocator->lock));
}



MODULE_POSTINIT(){
	fs_register_descriptor(&_fat32_filesystem_descriptor_config,&_fat32_filesystem_descriptor);
}
