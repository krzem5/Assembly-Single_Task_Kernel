#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/mmap/mmap.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kernel/writer/writer.h>
#define KERNEL_LOG_NAME "writer"



static omm_allocator_t* KERNEL_INIT_WRITE _writer_omm_allocator=NULL;



KERNEL_INIT(){
	LOG("Initializing file writer...");
	_writer_omm_allocator=omm_init("writer",sizeof(writer_t),8,4,pmm_alloc_counter("omm_writer"));
	spinlock_init(&(_writer_omm_allocator->lock));
}



KERNEL_PUBLIC writer_t* writer_init(vfs_node_t* node){
	panic("writer_init");
}



KERNEL_PUBLIC void writer_deinit(writer_t* writer){
	writer_flush(writer);
	panic("writer_deinit");
}



KERNEL_PUBLIC void writer_append(writer_t* writer,const void* data,u64 length){
	panic("writer_append");
}



KERNEL_PUBLIC void writer_flush(writer_t* writer){
	panic("writer_flush");
}
