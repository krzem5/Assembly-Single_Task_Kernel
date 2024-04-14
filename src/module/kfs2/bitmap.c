#include <kernel/drive/drive.h>
#include <kernel/fs/fs.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kfs2/crc.h>
#include <kfs2/structures.h>



static pmm_counter_descriptor_t* _kfs2_bitmap_buffer_pmm_counter=NULL;



static void _store_data(filesystem_t* fs,kfs2_bitmap_t* allocator,u32 level){
	kfs2_fs_extra_data_t* extra_data=fs->extra_data;
	partition_t* partition=fs->partition;
	drive_t* drive=partition->drive;
	if (drive_write(drive,partition->start_lba+((allocator->cache+level)->block_index<<extra_data->block_size_shift),(allocator->cache+level)->data,1<<extra_data->block_size_shift)!=(1<<extra_data->block_size_shift)){
		panic("_store_data: I/O error");
	}
}



static u64* _fetch_data(filesystem_t* fs,kfs2_bitmap_t* allocator,u32 level,u64 offset){
	u64 block_index=(allocator->cache+level)->offset+offset*sizeof(u64)/KFS2_BLOCK_SIZE;
	if ((allocator->cache+level)->block_index!=block_index){
		kfs2_fs_extra_data_t* extra_data=fs->extra_data;
		partition_t* partition=fs->partition;
		drive_t* drive=partition->drive;
		if (drive_read(drive,partition->start_lba+(block_index<<extra_data->block_size_shift),(allocator->cache+level)->data,1<<extra_data->block_size_shift)!=(1<<extra_data->block_size_shift)){
			panic("_fetch_data: I/O error");
		}
		(allocator->cache+level)->block_index=block_index;
	}
	return (allocator->cache+level)->data+(offset&(KFS2_BLOCK_SIZE/sizeof(u64)-1));
}



static void _find_next_highest_level_offset(filesystem_t* fs,kfs2_bitmap_t* allocator){
	for (;allocator->highest_level_offset<allocator->highest_level_length&&!(*_fetch_data(fs,allocator,KFS2_BITMAP_LEVEL_COUNT-1,allocator->highest_level_offset));allocator->highest_level_offset++);
}



MODULE_INIT(){
	_kfs2_bitmap_buffer_pmm_counter=pmm_alloc_counter("kfs2_bitmap_buffer");
}



void kfs2_bitmap_init(filesystem_t* fs,kfs2_bitmap_t* allocator,const u64* bitmap_offsets,u32 highest_level_length){
	spinlock_init(&(allocator->lock));
	allocator->highest_level_length=highest_level_length;
	allocator->highest_level_offset=0;
	for (u32 i=0;i<KFS2_BITMAP_LEVEL_COUNT;i++){
		(allocator->cache+i)->offset=bitmap_offsets[i];
		(allocator->cache+i)->block_index=0xffffffffffffffffull;
		(allocator->cache+i)->data=(void*)(pmm_alloc(1,_kfs2_bitmap_buffer_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	}
	_find_next_highest_level_offset(fs,allocator);
}



u64 kfs2_bitmap_alloc(filesystem_t* fs,kfs2_bitmap_t* allocator){
	spinlock_acquire_exclusive(&(allocator->lock));
	if (allocator->highest_level_offset==allocator->highest_level_length){
		spinlock_release_exclusive(&(allocator->lock));
		return 0;
	}
	u64 out=allocator->highest_level_offset;
	u64* ptr[KFS2_BITMAP_LEVEL_COUNT];
	for (u32 i=KFS2_BITMAP_LEVEL_COUNT;i;){
		i--;
		ptr[i]=_fetch_data(fs,allocator,i,out);
		out=(out<<6)|(__builtin_ffsll(*(ptr[i]))-1);
	}
	u32 i=0;
	for (;i<KFS2_BITMAP_LEVEL_COUNT;i++){
		*(ptr[i])&=(*(ptr[i]))-1;
		_store_data(fs,allocator,i);
		if (*(ptr[i])){
			break;
		}
	}
	if (i==KFS2_BITMAP_LEVEL_COUNT){
		allocator->highest_level_offset++;
		_find_next_highest_level_offset(fs,allocator);
	}
	spinlock_release_exclusive(&(allocator->lock));
	return out;
}
