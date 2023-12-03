#include <kernel/memory/amm.h>
#include <kernel/memory/smm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



#define FNV_OFFSET_BASIS 0x811c9dc5
#define FNV_PRIME 0x01000193



KERNEL_PUBLIC string_t* smm_alloc(const char* data,u32 length){
	if (!length){
		length=smm_length(data);
	}
	string_t* out=amm_alloc(sizeof(string_t)+length+1);
	out->length=length;
	out->hash=FNV_OFFSET_BASIS;
	for (u32 i=0;i<length;i++){
		out->data[i]=data[i];
		out->hash=(out->hash^data[i])*FNV_PRIME;
	}
	out->data[length]=0;
	return out;
}



KERNEL_PUBLIC void smm_dealloc(string_t* string){
	amm_dealloc(string);
}



KERNEL_PUBLIC string_t* smm_duplicate(const string_t* string){
	string_t* out=amm_alloc(sizeof(string_t)+string->length+1);
	memcpy(out,string,sizeof(string_t)+string->length+1);
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
