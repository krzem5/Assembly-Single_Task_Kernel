#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/memory/smm.h>



#define FNV_OFFSET_BASIS 0x811c9dc5
#define FNV_PRIME 0x01000193

#define DECLARE_ALLOCATOR(size) static omm_allocator_t _smm_allocator##size=OMM_ALLOCATOR_INIT_STRUCT("smm["#size"]",sizeof(string_t)+size,4,2,&_smm_omm_pmm_counter);
#define USE_ALLOCATOR(size) \
	if (length<(size)){ \
		out=omm_alloc(&_smm_allocator##size); \
	}
#define USE_ALLOCATOR_DEALLOC(size) \
	if (name->length<(size)){ \
		omm_dealloc(&_smm_allocator##size,name); \
	}



static pmm_counter_descriptor_t _smm_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_smm");
DECLARE_ALLOCATOR(4);
DECLARE_ALLOCATOR(8);
DECLARE_ALLOCATOR(12);
DECLARE_ALLOCATOR(16);
DECLARE_ALLOCATOR(24);
DECLARE_ALLOCATOR(32);
DECLARE_ALLOCATOR(48);
DECLARE_ALLOCATOR(64);
DECLARE_ALLOCATOR(96);
DECLARE_ALLOCATOR(128);
DECLARE_ALLOCATOR(192);
DECLARE_ALLOCATOR(256);



string_t* smm_alloc(const char* name,u32 length){
	if (!length){
		while (name[length]){
			length++;
		}
	}
	string_t* out;
	USE_ALLOCATOR(4)
	else USE_ALLOCATOR(8)
	else USE_ALLOCATOR(12)
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
		panic("smm_alloc: name too long");
	}
	out->length=length;
	out->hash=FNV_OFFSET_BASIS;
	if (name){
		for (u32 i=0;i<length;i++){
			out->data[i]=name[i];
			out->hash=(out->hash^name[i])*FNV_PRIME;
		}
	}
	else{
		memset(out->data,0,length);
	}
	out->data[length]=0;
	return out;
}



void smm_dealloc(string_t* name){
	USE_ALLOCATOR_DEALLOC(4)
	else USE_ALLOCATOR_DEALLOC(8)
	else USE_ALLOCATOR_DEALLOC(12)
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
		panic("smm_dealloc: name too long");
	}
}



string_t* smm_duplicate(const string_t* name){
	u32 length=name->length;
	string_t* out;
	USE_ALLOCATOR(4)
	else USE_ALLOCATOR(8)
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
		panic("smm_alloc: name too long");
	}
	memcpy(out,name,sizeof(string_t)+length+1);
	return out;
}



void smm_rehash(string_t* name){
	name->hash=FNV_OFFSET_BASIS;
	for (u32 i=0;i<name->length;i++){
		name->hash=(name->hash^name->data[i])*FNV_PRIME;
	}
}



void _smm_cleanup(string_t** string){
	smm_dealloc(*string);
}
