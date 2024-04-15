#ifndef _KERNEL_WRITER_WRITER_H_
#define _KERNEL_WRITER_WRITER_H_ 1
#include <kernel/lock/spinlock.h>
#include <kernel/mmap/mmap.h>
#include <kernel/types.h>
#include <kernel/vfs/node.h>



typedef struct _WRITER{
	spinlock_t lock;
	vfs_node_t* node;
	mmap_region_t* buffer_region;
	void* buffer;
	u64 offset;
	u64 size;
} writer_t;



writer_t* writer_init(vfs_node_t* node);



void writer_deinit(writer_t* writer);



void writer_append(writer_t* writer,const void* data,u64 length);



void writer_flush(writer_t* writer);



static KERNEL_INLINE void writer_append_u8(writer_t* writer,u8 value){
	writer_append(writer,&value,sizeof(value));
}



static KERNEL_INLINE void writer_append_u16(writer_t* writer,u16 value){
	writer_append(writer,&value,sizeof(value));
}



static KERNEL_INLINE void writer_append_u32(writer_t* writer,u32 value){
	writer_append(writer,&value,sizeof(value));
}



static KERNEL_INLINE void writer_append_u64(writer_t* writer,u64 value){
	writer_append(writer,&value,sizeof(value));
}



#endif
