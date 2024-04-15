#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/mp/process.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kernel/writer/writer.h>
#define KERNEL_LOG_NAME "writer"



#define WRITER_BUFFER_SIZE 2097152 // power of 2



static omm_allocator_t* KERNEL_INIT_WRITE _writer_omm_allocator=NULL;



static void _emit_data(writer_t* writer,const void* buffer,u64 size){
	writer->size+=size;
	vfs_node_resize(writer->node,writer->size,0);
	vfs_node_write(writer->node,writer->size-size,buffer,size,0);
}



KERNEL_INIT(){
	LOG("Initializing file writer...");
	_writer_omm_allocator=omm_init("writer",sizeof(writer_t),8,4,pmm_alloc_counter("omm_writer"));
	spinlock_init(&(_writer_omm_allocator->lock));
}



KERNEL_PUBLIC writer_t* writer_init(vfs_node_t* node){
	writer_t* out=omm_alloc(_writer_omm_allocator);
	spinlock_init(&(out->lock));
	out->node=node;
	out->buffer_region=mmap_alloc(process_kernel->mmap,0,WRITER_BUFFER_SIZE,MMAP_REGION_FLAG_VMM_WRITE,NULL);
	out->buffer=(void*)(out->buffer_region->rb_node.key);
	out->offset=0;
	out->size=0;
	return out;
}



KERNEL_PUBLIC u64 writer_deinit(writer_t* writer){
	if (writer->offset){
		_emit_data(writer,writer->buffer,writer->offset);
	}
	mmap_dealloc_region(process_kernel->mmap,writer->buffer_region);
	u64 out=writer->size;
	omm_dealloc(_writer_omm_allocator,writer);
	return out;
}



KERNEL_PUBLIC void writer_append(writer_t* writer,const void* data,u64 length){
	if (!length){
		return;
	}
	spinlock_acquire_exclusive(&(writer->lock));
	u64 space=WRITER_BUFFER_SIZE-writer->offset;
	if (space>length){
		space=length;
	}
	mem_copy(writer->buffer+writer->offset,data,space);
	data+=space;
	length-=space;
	writer->offset+=space;
	if (writer->offset<WRITER_BUFFER_SIZE){
		goto _skip_flush;
	}
	_emit_data(writer,writer->buffer,WRITER_BUFFER_SIZE);
	writer->offset=0;
	if (length>=WRITER_BUFFER_SIZE){
		space=length&(-WRITER_BUFFER_SIZE);
		_emit_data(writer,data,space);
		data+=space;
		length-=space;
	}
	mem_copy(writer->buffer,data,length);
	writer->offset=length;
_skip_flush:
	spinlock_release_exclusive(&(writer->lock));
}



KERNEL_PUBLIC void writer_flush(writer_t* writer){
	spinlock_acquire_exclusive(&(writer->lock));
	if (writer->offset){
		_emit_data(writer,writer->buffer,writer->offset);
		writer->offset=0;
	}
	spinlock_release_exclusive(&(writer->lock));
}
