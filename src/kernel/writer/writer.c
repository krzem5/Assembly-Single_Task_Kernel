#include <kernel/exception/exception.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
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



static KERNEL_AWAITS void _emit_data(writer_t* writer,const void* buffer,u64 size){
	writer->size+=size;
	if (writer->is_file_backed){
		vfs_node_resize(writer->backend.node,writer->size,0);
		vfs_node_write(writer->backend.node,writer->size-size,buffer,size,0);
	}
	else{
		*(writer->backend.buffer)=amm_realloc(*(writer->backend.buffer),writer->size);
		mem_copy((*(writer->backend.buffer))+writer->size-size,buffer,size);
	}
}



KERNEL_INIT(){
	_writer_omm_allocator=omm_init("kernel.writer",sizeof(writer_t),8,4);
	rwlock_init(&(_writer_omm_allocator->lock));
}



KERNEL_PUBLIC KERNEL_NO_AWAITS writer_t* writer_init(vfs_node_t* node,void** buffer){
	writer_t* out=omm_alloc(_writer_omm_allocator);
	rwlock_init(&(out->lock));
	if (node){
		out->is_file_backed=1;
		out->backend.node=node;
	}
	else{
		out->is_file_backed=0;
		out->backend.buffer=buffer;
	}
	out->buffer_region=mmap_alloc(process_kernel->mmap,0,WRITER_BUFFER_SIZE,MMAP_REGION_FLAG_VMM_WRITE,NULL);
	out->buffer=(void*)(out->buffer_region->rb_node.key);
	out->offset=0;
	out->size=0;
	return out;
}



KERNEL_PUBLIC KERNEL_AWAITS u64 writer_deinit(writer_t* writer){
	exception_unwind_push(writer){
		writer_deinit_exception(EXCEPTION_UNWIND_ARG(0));
	}
	if (writer->offset){
		_emit_data(writer,writer->buffer,writer->offset);
	}
	exception_unwind_pop();
	mmap_dealloc_region(process_kernel->mmap,writer->buffer_region);
	u64 out=writer->size;
	omm_dealloc(_writer_omm_allocator,writer);
	return out;
}



KERNEL_PUBLIC void writer_deinit_exception(writer_t* writer){
	mmap_dealloc_region(process_kernel->mmap,writer->buffer_region);
	omm_dealloc(_writer_omm_allocator,writer);
}



KERNEL_PUBLIC KERNEL_AWAITS void writer_append(writer_t* writer,const void* data,u64 length){
	if (!length){
		return;
	}
	rwlock_acquire_write(&(writer->lock));
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
	rwlock_release_write(&(writer->lock));
}



KERNEL_PUBLIC KERNEL_AWAITS void writer_flush(writer_t* writer){
	rwlock_acquire_write(&(writer->lock));
	if (writer->offset){
		_emit_data(writer,writer->buffer,writer->offset);
		writer->offset=0;
	}
	rwlock_release_write(&(writer->lock));
}
