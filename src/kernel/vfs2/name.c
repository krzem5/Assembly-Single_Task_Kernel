#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs2/name.h>



#define FNV_OFFSET_BASIS 0x811c9dc5
#define FNV_PRIME 0x01000193

#define DECLARE_ALLOCATOR(size) static omm_allocator_t _vfs2_name_allocator##size=OMM_ALLOCATOR_INIT_STRUCT("vfs2_name["#size"]",sizeof(vfs2_node_name_t)+8,8,4,PMM_COUNTER_OMM_VFS2_NAME);
#define USE_ALLOCATOR(size) \
	if (length<(size)){ \
		out=omm_alloc(&_vfs2_name_allocator##size); \
	}
#define USE_ALLOCATOR_DEALLOC(size) \
	if (name->length<(size)){ \
		omm_dealloc(&_vfs2_name_allocator##size,name); \
	}



PMM_DECLARE_COUNTER(OMM_VFS2_NAME);



DECLARE_ALLOCATOR(8);
DECLARE_ALLOCATOR(16);
DECLARE_ALLOCATOR(24);
DECLARE_ALLOCATOR(32);
DECLARE_ALLOCATOR(48);
DECLARE_ALLOCATOR(64);
DECLARE_ALLOCATOR(96);
DECLARE_ALLOCATOR(128);
DECLARE_ALLOCATOR(192);
DECLARE_ALLOCATOR(256);



vfs2_node_name_t* vfs2_name_alloc(const char* name,u32 length){
	if (!length){
		while (name[length]){
			length++;
		}
	}
	vfs2_node_name_t* out;
	USE_ALLOCATOR(8)
	else USE_ALLOCATOR(16)
	else USE_ALLOCATOR(24)
	else USE_ALLOCATOR(32)
	else USE_ALLOCATOR(48)
	else USE_ALLOCATOR(64)
	else USE_ALLOCATOR(96)
	else USE_ALLOCATOR(128)
	else USE_ALLOCATOR(192)
	else USE_ALLOCATOR(256)
	else{
		panic("vfs2_name_alloc: name too long");
	}
	out->length=length;
	out->hash=FNV_OFFSET_BASIS;
	for (u32 i=0;i<length;i++){
		out->data[i]=name[i];
		out->hash=(out->hash^name[i])*FNV_PRIME;
	}
	out->data[length]=0;
	return out;
}



void vfs2_name_dealloc(vfs2_node_name_t* name){
	USE_ALLOCATOR_DEALLOC(8)
	else USE_ALLOCATOR_DEALLOC(16)
	else USE_ALLOCATOR_DEALLOC(24)
	else USE_ALLOCATOR_DEALLOC(32)
	else USE_ALLOCATOR_DEALLOC(48)
	else USE_ALLOCATOR_DEALLOC(64)
	else USE_ALLOCATOR_DEALLOC(96)
	else USE_ALLOCATOR_DEALLOC(128)
	else USE_ALLOCATOR_DEALLOC(192)
	else USE_ALLOCATOR_DEALLOC(256)
	else{
		panic("vfs2_name_dealloc: name too long");
	}
}



vfs2_node_name_t* vfs2_name_duplicate(const vfs2_node_name_t* name){
	u32 length=name->length;
	vfs2_node_name_t* out;
	USE_ALLOCATOR(8)
	else USE_ALLOCATOR(16)
	else USE_ALLOCATOR(24)
	else USE_ALLOCATOR(32)
	else USE_ALLOCATOR(48)
	else USE_ALLOCATOR(64)
	else USE_ALLOCATOR(96)
	else USE_ALLOCATOR(128)
	else USE_ALLOCATOR(192)
	else USE_ALLOCATOR(256)
	else{
		panic("vfs2_name_alloc: name too long");
	}
	memcpy(out,name,sizeof(vfs2_node_name_t)+length+1);
	return out;
}
