#include <devfs/fs.h>
#include <kernel/format/format.h>
#include <kernel/fs/fs.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "devfs_fs"



typedef struct _DEVFS_VFS_NODE{
	vfs_node_t node;
	string_t* data;
} devfs_vfs_node_t;



static pmm_counter_descriptor_t _devfs_node_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_devfs_node");
static omm_allocator_t _devfs_vfs_node_allocator=OMM_ALLOCATOR_INIT_STRUCT("devfs_node",sizeof(devfs_vfs_node_t),8,1,&_devfs_node_omm_pmm_counter);



filesystem_t* devfs;



static vfs_node_t* _devfs_create(void){
	devfs_vfs_node_t* out=omm_alloc(&_devfs_vfs_node_allocator);
	out->data=NULL;
	return (vfs_node_t*)out;
}



static s64 _devfs_read(vfs_node_t* node,u64 offset,void* buffer,u64 size){
	devfs_vfs_node_t* devfs_node=(devfs_vfs_node_t*)node;
	if (!devfs_node->data||offset>=devfs_node->data->length){
		return 0;
	}
	if (offset+size>devfs_node->data->length){
		size=devfs_node->data->length-offset;
	}
	memcpy(buffer,devfs_node->data->data+offset,size);
	return size;
}



static s64 _devfs_resize(vfs_node_t* node,s64 size,u32 flags){
	devfs_vfs_node_t* devfs_node=(devfs_vfs_node_t*)node;
	if (!devfs_node->data){
		return 0;
	}
	if ((flags&VFS_NODE_FLAG_RESIZE_RELATIVE)&&!size){
		return devfs_node->data->length;
	}
	return 0;
}



static const vfs_functions_t _devfs_functions={
	_devfs_create,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	_devfs_read,
	NULL,
	_devfs_resize,
	NULL
};



static filesystem_descriptor_t _devfs_filesystem_descriptor={
	"devfs",
	NULL,
	NULL
};



void devfs_create_fs(void){
	LOG("Creating devfs filesystem...");
	fs_register_descriptor(&_devfs_filesystem_descriptor);
	devfs=fs_create(&_devfs_filesystem_descriptor);
	devfs->functions=&_devfs_functions;
	devfs->root=devfs_create_node(NULL,"",NULL);
	vfs_mount(devfs,"/dev");
}



vfs_node_t* devfs_create_node(vfs_node_t* parent,const char* name,string_t* data){
	SMM_TEMPORARY_STRING name_string=smm_alloc(name,0);
	devfs_vfs_node_t* out=(devfs_vfs_node_t*)vfs_node_create(devfs,name_string);
	out->node.flags|=VFS_NODE_FLAG_VIRTUAL;
	if (data){
		out->node.flags|=VFS_NODE_TYPE_FILE;
		out->data=data;
	}
	else{
		out->node.flags|=VFS_NODE_TYPE_DIRECTORY;
		out->data=NULL;
	}
	if (parent){
		vfs_node_attach_external_child(parent,(vfs_node_t*)out);
	}
	return (vfs_node_t*)out;
}



void devfs_create_data_node(vfs_node_t* parent,const char* name,const char* format,...){
	__builtin_va_list va;
	__builtin_va_start(va,format);
	char buffer[256];
	devfs_create_node(parent,name,smm_alloc(buffer,format_string_va(buffer,256,format,&va)));
	__builtin_va_end(va);
}



void devfs_create_link_node(vfs_node_t* parent,const char* name,const char* format,...){
	__builtin_va_list va;
	__builtin_va_start(va,format);
	char buffer[256];
	vfs_node_t* node=devfs_create_node(parent,name,smm_alloc(buffer,format_string_va(buffer,256,format,&va)));
	node->flags=(node->flags&(~VFS_NODE_TYPE_MASK))|VFS_NODE_TYPE_LINK;
	__builtin_va_end(va);
}
