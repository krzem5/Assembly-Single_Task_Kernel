#include <dynamicfs/dynamicfs.h>
#include <kernel/format/format.h>
#include <kernel/fs/fs.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/util/memory.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>



typedef struct _DYNAMICFS_VFS_NODE{
	vfs_node_t node;
	string_t* data;
	dynamicfs_read_callback_t read_callback;
	void* read_callback_ctx;
} dynamicfs_vfs_node_t;



static omm_allocator_t* _dynamicfs_vfs_node_allocator=NULL;



static vfs_node_t* _dynamicfs_create(void){
	dynamicfs_vfs_node_t* out=omm_alloc(_dynamicfs_vfs_node_allocator);
	out->data=NULL;
	out->read_callback=NULL;
	out->read_callback_ctx=NULL;
	return (vfs_node_t*)out;
}



static void _dynamicfs_delete(vfs_node_t* node){
	omm_dealloc(_dynamicfs_vfs_node_allocator,node);
}



static u64 _dynamicfs_read(vfs_node_t* node,u64 offset,void* buffer,u64 size,u32 flags){
	if (!size){
		return 0;
	}
	dynamicfs_vfs_node_t* dynamicfs_node=(dynamicfs_vfs_node_t*)node;
	if (dynamicfs_node->read_callback){
		return dynamicfs_node->read_callback(dynamicfs_node->read_callback_ctx,offset,buffer,size);
	}
	if (!dynamicfs_node->data||offset>=dynamicfs_node->data->length){
		return 0;
	}
	if (offset+size>dynamicfs_node->data->length){
		size=dynamicfs_node->data->length-offset;
	}
	mem_copy(buffer,dynamicfs_node->data->data+offset,size);
	return size;
}



static u64 _dynamicfs_resize(vfs_node_t* node,s64 size,u32 flags){
	dynamicfs_vfs_node_t* dynamicfs_node=(dynamicfs_vfs_node_t*)node;
	if (!dynamicfs_node->data&&!dynamicfs_node->read_callback){
		return 0;
	}
	if ((flags&VFS_NODE_FLAG_RESIZE_RELATIVE)&&!size){
		return (dynamicfs_node->data?dynamicfs_node->data->length:dynamicfs_node->read_callback(dynamicfs_node->read_callback_ctx,0,NULL,0));
	}
	return 0;
}



static const vfs_functions_t _dynamicfs_functions={
	_dynamicfs_create,
	_dynamicfs_delete,
	NULL,
	NULL,
	NULL,
	NULL,
	_dynamicfs_read,
	NULL,
	_dynamicfs_resize,
	NULL
};



KERNEL_PUBLIC filesystem_t* dynamicfs_init(const char* path,const filesystem_descriptor_config_t* fs_descriptor_config){
	if (!_dynamicfs_vfs_node_allocator){
		_dynamicfs_vfs_node_allocator=omm_init("dynamicfs_node",sizeof(dynamicfs_vfs_node_t),8,2,pmm_alloc_counter("omm_dynamicfs_node"));
		spinlock_init(&(_dynamicfs_vfs_node_allocator->lock));
	}
	filesystem_t* out=fs_create(fs_register_descriptor(fs_descriptor_config));
	out->functions=&_dynamicfs_functions;
	SMM_TEMPORARY_STRING name_string=smm_alloc("",0);
	out->root=vfs_node_create(out,name_string);
	out->root->flags|=VFS_NODE_FLAG_VIRTUAL|VFS_NODE_TYPE_DIRECTORY|(0555<<VFS_NODE_PERMISSION_SHIFT);
	vfs_mount(out,path,0);
	return out;
}



KERNEL_PUBLIC vfs_node_t* dynamicfs_create_node(vfs_node_t* parent,const char* name,u32 type,string_t* data,dynamicfs_read_callback_t read_callback,void* read_callback_ctx){
	SMM_TEMPORARY_STRING name_string=smm_alloc(name,0);
	dynamicfs_vfs_node_t* out=(dynamicfs_vfs_node_t*)vfs_node_create(parent->fs,name_string);
	out->node.flags|=VFS_NODE_FLAG_VIRTUAL|type|((type==VFS_NODE_TYPE_DIRECTORY||type==VFS_NODE_TYPE_LINK?0555:0444)<<VFS_NODE_PERMISSION_SHIFT);
	out->data=data;
	out->read_callback=read_callback;
	out->read_callback_ctx=read_callback_ctx;
	vfs_node_attach_child(parent,(vfs_node_t*)out);
	return (vfs_node_t*)out;
}



KERNEL_PUBLIC vfs_node_t* dynamicfs_create_data_node(vfs_node_t* parent,const char* name,const char* format,...){
	__builtin_va_list va;
	__builtin_va_start(va,format);
	char buffer[256];
	vfs_node_t* out=dynamicfs_create_node(parent,name,VFS_NODE_TYPE_FILE,smm_alloc(buffer,format_string_va(buffer,256,format,&va)),NULL,NULL);
	__builtin_va_end(va);
	return out;
}



KERNEL_PUBLIC vfs_node_t* dynamicfs_create_link_node(vfs_node_t* parent,const char* name,const char* format,...){
	__builtin_va_list va;
	__builtin_va_start(va,format);
	char buffer[256];
	vfs_node_t* out=dynamicfs_create_node(parent,name,VFS_NODE_TYPE_LINK,smm_alloc(buffer,format_string_va(buffer,256,format,&va)),NULL,NULL);
	__builtin_va_end(va);
	return out;
}



KERNEL_PUBLIC void dynamicfs_delete_node(vfs_node_t* node,_Bool delete_string){
	dynamicfs_vfs_node_t* dynamicfs_node=(dynamicfs_vfs_node_t*)node;
	string_t* string=dynamicfs_node->data;
	dynamicfs_node->data=NULL;
	dynamicfs_node->read_callback=NULL;
	dynamicfs_node->read_callback_ctx=NULL;
	if (delete_string&&string){
		smm_dealloc(string);
	}
	vfs_node_dettach_child(node);
	vfs_node_delete(node);
}



KERNEL_PUBLIC u64 dynamicfs_process_simple_read(const void* data,u64 length,u64 offset,void* buffer,u64 size){
	if (!buffer){
		return length;
	}
	if (offset>=length){
		return 0;
	}
	if (offset+size>length){
		size=length-offset;
	}
	mem_copy(buffer,data+offset,size);
	return size;
}



KERNEL_PUBLIC u64 dynamicfs_integer_read_callback(void* ctx,u64 offset,void* buffer,u64 size){
	char ret[32];
	return dynamicfs_process_simple_read(ret,format_string(ret,32,"%lu",*((const u64*)ctx)),offset,buffer,size);
}



KERNEL_PUBLIC u64 dynamicfs_string_read_callback(void* ctx,u64 offset,void* buffer,u64 size){
	const string_t* string=*((const string_t*const*)ctx);
	return dynamicfs_process_simple_read((string?string->data:""),(string?string->length:0),offset,buffer,size);
}
