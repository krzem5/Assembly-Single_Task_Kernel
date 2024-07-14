#ifndef _COMMON_FAT32_API_H_
#define _COMMON_FAT32_API_H_ 1
#include <common/fat32/util.h>



#define FAT32_SECTOR_SIZE 512

#define FAT32_NODE_FLAG_READONLY 0x01
#define FAT32_NODE_FLAG_DIRECTORY 0x02



typedef void* (*fat32_filesystem_page_alloc_callback_t)(u64);



typedef void (*fat32_filesystem_page_dealloc_callback_t)(void*,u64);



typedef u64 (*fat32_filesystem_block_read_callback_t)(void*,u64,void*,u64);



typedef u64 (*fat32_filesystem_block_write_callback_t)(void*,u64,const void*,u64);



typedef struct _FAT32_TIME{
	u16 year;
	u8 month;
	u8 day;
	u8 hour;
	u8 minute;
	u8 second;
	u32 nanosecond;
} fat32_time_t;



typedef struct _FAT32_NODE{
	u64 pointer;
	u32 flags;
	u32 cluster;
	u32 size;
	fat32_time_t time_access;
	fat32_time_t time_modify;
	fat32_time_t time_birth;
} fat32_node_t;



typedef struct _FAT32_FILESYSTEM_CONFIG{
	void* ctx;
	fat32_filesystem_block_read_callback_t read_callback;
	fat32_filesystem_block_write_callback_t write_callback;
	fat32_filesystem_page_alloc_callback_t alloc_callback;
	fat32_filesystem_page_dealloc_callback_t dealloc_callback;
	u64 block_size;
	u64 start_lba;
	u64 end_lba;
} fat32_filesystem_config_t;



typedef struct _FAT32_FILESYSTEM{
	fat32_filesystem_config_t config;
	u32 cluster_size;
	u32 reserved_sectors;
	u32 fat_count;
	u32 sector_count;
	u32 sectors_per_fat;
	u32 root_directory_cluster;
	u32 first_cluster_lba;
	u64* fat_bitmap;
	u32* fat_data;
} fat32_filesystem_t;



bool fat32_filesystem_format(const fat32_filesystem_config_t* config,fat32_filesystem_t* out);



bool fat32_filesystem_init(const fat32_filesystem_config_t* config,fat32_filesystem_t* out);



void fat32_filesystem_deinit(fat32_filesystem_t* fs);



bool fat32_filesystem_get_root(fat32_filesystem_t* fs,fat32_node_t* out);



bool fat32_node_create(fat32_filesystem_t* fs,fat32_node_t* parent,const char* name,u32 name_length,u32 flags,fat32_node_t* out);



bool fat32_node_delete(fat32_filesystem_t* fs,fat32_node_t* node);



bool fat32_node_lookup(fat32_filesystem_t* fs,fat32_node_t* parent,const char* name,u32 name_length,fat32_node_t* out);



u64 fat32_node_iterate(fat32_filesystem_t* fs,fat32_node_t* parent,u64 pointer,char* buffer,u32* buffer_length);



bool fat32_node_link(fat32_filesystem_t* fs,fat32_node_t* parent,fat32_node_t* child,const char* name,u32 name_length);



bool fat32_node_unlink(fat32_filesystem_t* fs,fat32_node_t* parent,fat32_node_t* child,const char* name,u32 name_length);



u64 fat32_node_read(fat32_filesystem_t* fs,fat32_node_t* node,u64 offset,void* buffer,u64 size);



u64 fat32_node_write(fat32_filesystem_t* fs,fat32_node_t* node,u64 offset,const void* buffer,u64 size);



u64 fat32_node_resize(fat32_filesystem_t* fs,fat32_node_t* node,u64 size);



bool fat32_node_flush(fat32_filesystem_t* fs,fat32_node_t* node);



#endif
