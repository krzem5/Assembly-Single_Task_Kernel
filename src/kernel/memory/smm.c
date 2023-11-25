#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "smm"



#define FNV_OFFSET_BASIS 0x811c9dc5
#define FNV_PRIME 0x01000193

#define DECLARE_ALLOCATOR(size) static omm_allocator_t* _smm_allocator##size=NULL;
#define USE_ALLOCATOR(size) \
	if (length<(size)){ \
		if (!_smm_allocator##size){ \
			_smm_allocator##size=omm_init("smm["#size"]",sizeof(string_t)+size,4,2,_smm_omm_pmm_counter); \
			spinlock_init(&(_smm_allocator##size->lock)); \
		} \
		out=omm_alloc(_smm_allocator##size); \
	}
#define USE_ALLOCATOR_DEALLOC(size) \
	if (string->length<(size)){ \
		omm_dealloc(_smm_allocator##size,string); \
	}



static pmm_counter_descriptor_t* _smm_omm_pmm_counter=NULL;
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
DECLARE_ALLOCATOR(384);
DECLARE_ALLOCATOR(512);
DECLARE_ALLOCATOR(768);
DECLARE_ALLOCATOR(1024);
DECLARE_ALLOCATOR(2048);



KERNEL_PUBLIC string_t* smm_alloc(const char* data,u32 length){
	if (!_smm_omm_pmm_counter){
		_smm_omm_pmm_counter=pmm_alloc_counter("omm_smm");
	}
	if (!length){
		length=smm_length(data);
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
	else USE_ALLOCATOR(384)
	else USE_ALLOCATOR(512)
	else USE_ALLOCATOR(768)
	else USE_ALLOCATOR(1024)
	else USE_ALLOCATOR(2048)
	else{
		panic("smm_alloc: string too long");
	}
	out->length=length;
	out->hash=FNV_OFFSET_BASIS;
	if (data){
		for (u32 i=0;i<length;i++){
			out->data[i]=data[i];
			out->hash=(out->hash^data[i])*FNV_PRIME;
		}
	}
	else{
		memset(out->data,0,length);
	}
	out->data[length]=0;
	return out;
}



KERNEL_PUBLIC void smm_dealloc(string_t* string){
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
	else USE_ALLOCATOR_DEALLOC(384)
	else USE_ALLOCATOR_DEALLOC(512)
	else USE_ALLOCATOR_DEALLOC(768)
	else USE_ALLOCATOR_DEALLOC(1024)
	else USE_ALLOCATOR_DEALLOC(2048)
	else{
		panic("smm_dealloc: string too long");
	}
}



KERNEL_PUBLIC string_t* smm_duplicate(const string_t* string){
	u32 length=string->length;
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
	else USE_ALLOCATOR(384)
	else USE_ALLOCATOR(512)
	else USE_ALLOCATOR(768)
	else USE_ALLOCATOR(1024)
	else USE_ALLOCATOR(2048)
	else{
		panic("smm_alloc: string too long");
	}
	memcpy(out,string,sizeof(string_t)+length+1);
	return out;
}



KERNEL_PUBLIC void smm_rehash(string_t* string){
	string->hash=FNV_OFFSET_BASIS;
	for (u32 i=0;i<string->length;i++){
		string->hash=(string->hash^string->data[i])*FNV_PRIME;
	}
}



KERNEL_PUBLIC u32 smm_length(const char* data){
	u32 out=0;
	for (;data[out];out++);
	return out;
}



KERNEL_PUBLIC void _smm_cleanup(string_t** string){
	smm_dealloc(*string);
}
