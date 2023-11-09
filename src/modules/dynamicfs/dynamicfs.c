#include <dynamicfs/dynamicfs.h>
#include <kernel/format/format.h>
#include <kernel/fs/fs.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>



typedef struct _DYNAMICFS_VFS_NODE{
	vfs_node_t node;
	string_t* data;
	dynamicfs_read_callback_t read_callback;
	void* read_callback_ctx;
} dynamicfs_vfs_node_t;



static pmm_counter_descriptor_t _dynamicfs_node_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_dynamicfs_node");
static omm_allocator_t _dynamicfs_vfs_node_allocator=OMM_ALLOCATOR_INIT_STRUCT("dynamicfs_node",sizeof(dynamicfs_vfs_node_t),8,2,&_dynamicfs_node_omm_pmm_counter);



static vfs_node_t* _dynamicfs_create(void){
	dynamicfs_vfs_node_t* out=omm_alloc(&_dynamicfs_vfs_node_allocator);
	out->data=NULL;
	out->read_callback=NULL;
	out->read_callback_ctx=NULL;
	return (vfs_node_t*)out;
}



static s64 _dynamicfs_read(vfs_node_t* node,u64 offset,void* buffer,u64 size){
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
	memcpy(buffer,dynamicfs_node->data->data+offset,size);
	return size;
}



static s64 _dynamicfs_resize(vfs_node_t* node,s64 size,u32 flags){
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
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	_dynamicfs_read,
	NULL,
	_dynamicfs_resize,
	NULL
};



filesystem_t* dynamicfs_init(const char* path,filesystem_descriptor_t* fs_descriptor){
	fs_register_descriptor(fs_descriptor);
	filesystem_t* out=fs_create(fs_descriptor);
	out->functions=&_dynamicfs_functions;
	SMM_TEMPORARY_STRING name_string=smm_alloc("",0);
	out->root=vfs_node_create(out,name_string);
	out->root->flags|=VFS_NODE_FLAG_VIRTUAL|VFS_NODE_TYPE_DIRECTORY;
	vfs_mount(out,path);
	return out;
}



vfs_node_t* dynamicfs_create_node(vfs_node_t* parent,const char* name,u32 type,string_t* data,dynamicfs_read_callback_t read_callback,void* read_callback_ctx){
	SMM_TEMPORARY_STRING name_string=smm_alloc(name,0);
	dynamicfs_vfs_node_t* out=(dynamicfs_vfs_node_t*)vfs_node_create(parent->fs,name_string);
	out->node.flags|=VFS_NODE_FLAG_VIRTUAL|type;
	out->data=data;
	out->read_callback=read_callback;
	out->read_callback_ctx=read_callback_ctx;
	vfs_node_attach_external_child(parent,(vfs_node_t*)out);
	return (vfs_node_t*)out;
}



vfs_node_t* dynamicfs_create_data_node(vfs_node_t* parent,const char* name,const char* format,...){
	__builtin_va_list va;
	__builtin_va_start(va,format);
	char buffer[256];
	vfs_node_t* out=dynamicfs_create_node(parent,name,VFS_NODE_TYPE_FILE,smm_alloc(buffer,format_string_va(buffer,256,format,&va)),NULL,NULL);
	__builtin_va_end(va);
	return out;
}



vfs_node_t* dynamicfs_create_link_node(vfs_node_t* parent,const char* name,const char* format,...){
	__builtin_va_list va;
	__builtin_va_start(va,format);
	char buffer[256];
	vfs_node_t* out=dynamicfs_create_node(parent,name,VFS_NODE_TYPE_LINK,smm_alloc(buffer,format_string_va(buffer,256,format,&va)),NULL,NULL);
	__builtin_va_end(va);
	return out;
}



u64 dynamicfs_integer_read_callback(void* ctx,u64 offset,void* buffer,u64 size){
	char tmp[21];
	u64 length=format_string(tmp,21,"%lu",*((u64*)ctx));
	if (offset>=length){
		return 0;
	}
	if (offset+size>length){
		size=length-offset;
	}
	memcpy(buffer,tmp+offset,size);
	return size;
}
