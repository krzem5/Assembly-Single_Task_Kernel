#include <kernel/config/config.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/mp/process.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "config"



#define CONFIG_FILE_SIGNATURE 0x80474643

#define CONFIG_FILE_VERSION 1

#define COFNIG_FILE_FLAG_COMPRESSED 1
#define COFNIG_FILE_FLAG_ENCRYPTED 2



typedef struct KERNEL_PACKED _CONFIG_FILE_HEADER{
	u32 signature;
	u8 version;
	u8 flags;
	u32 size;
	u8 padding[6];
} config_file_header_t;



typedef struct KERNEL_PACKED _CONFIG_FILE_ENCRYPTION_HEADER{
	u8 hmac[32];
	u8 salt[16];
} config_file_encryption_header_t;



typedef struct KERNEL_PACKED _CONFIG_FILE_TAG_HEADER{
	u8 type;
	u8 name_length;
	u16 length;
} config_file_tag_header_t;



static pmm_counter_descriptor_t* _config_buffer_pmm_counter=NULL;
static omm_allocator_t* _config_tag_allocator=NULL;



static const void* _parse_tag(const void* data,const void* end,config_tag_t** out){
	if (data+sizeof(config_file_tag_header_t)>end){
		return NULL;
	}
	const config_file_tag_header_t* header=data;
	data+=sizeof(config_file_tag_header_t);
	if (data+header->name_length+(header->length==0xffff?sizeof(u32):0)>end){
		return NULL;
	}
	*out=omm_alloc(_config_tag_allocator);
	(*out)->name=smm_alloc(data,header->name_length);
	(*out)->type=header->type;
	data+=header->name_length;
	u32 tag_length=header->length;
	if (tag_length==0xffff){
		tag_length=*((const u32*)data);
		data+=sizeof(u32);
	}
	if (header->type==CONFIG_TAG_TYPE_ARRAY){
		(*out)->array=amm_alloc(sizeof(config_tag_array_t)+sizeof(config_tag_t*)*tag_length);
		(*out)->array->length=tag_length;
		memset((*out)->array->data,0,sizeof(config_tag_t*)*tag_length);
		for (u32 i=0;i<tag_length;i++){
			data=_parse_tag(data,end,(*out)->array->data+i);
			if (!data){
				goto _error;
			}
		}
	}
	else if (header->type==CONFIG_TAG_TYPE_STRING){
		(*out)->string=NULL;
		if (data+tag_length>end){
			goto _error;
		}
		(*out)->string=smm_alloc(data,tag_length);
		data+=tag_length;
	}
	else if (header->type==CONFIG_TAG_TYPE_INT||header->type==CONFIG_TAG_TYPE_INT_NEGATIVE){
		if (data+tag_length>end){
			goto _error;
		}
		s64 value=0;
		for (u32 i=0;i<tag_length;i++){
			value|=((u64)(*((const u8*)(data+i))))<<(i<<3);
		}
		data+=tag_length;
		if (header->type==CONFIG_TAG_TYPE_INT_NEGATIVE){
			(*out)->type=CONFIG_TAG_TYPE_INT;
			value=-value;
		}
		(*out)->int_=value;
	}
	return data;
_error:
	config_tag_delete(*out);
	return NULL;
}



KERNEL_INIT(){
	LOG("Initializing config tags...");
	_config_buffer_pmm_counter=pmm_alloc_counter("config_buffer");
	_config_tag_allocator=omm_init("config_tag",sizeof(config_tag_t),8,4,pmm_alloc_counter("omm_config_tag"));
	spinlock_init(&(_config_tag_allocator->lock));
}



KERNEL_PUBLIC config_tag_t* config_tag_create(u32 type,const char* name,u8 name_length){
	config_tag_t* out=omm_alloc(_config_tag_allocator);
	out->name=smm_alloc(name,name_length);
	out->type=type;
	if (type==CONFIG_TAG_TYPE_ARRAY){
		out->array=amm_alloc(sizeof(config_tag_array_t));
		out->array->length=0;
	}
	else if (type==CONFIG_TAG_TYPE_STRING){
		out->string=smm_alloc("",0);
	}
	else if (type==CONFIG_TAG_TYPE_INT||type==CONFIG_TAG_TYPE_INT_NEGATIVE){
		out->int_=0;
	}
	return out;
}



KERNEL_PUBLIC void config_tag_delete(config_tag_t* tag){
	if (!tag){
		return;
	}
	if (tag->type==CONFIG_TAG_TYPE_ARRAY){
		for (u32 i=0;i<tag->array->length;i++){
			config_tag_delete(tag->array->data[i]);
		}
		amm_dealloc(tag->array);
	}
	else if (tag->type==CONFIG_TAG_TYPE_STRING){
		smm_dealloc(tag->string);
	}
	smm_dealloc(tag->name);
	omm_dealloc(_config_tag_allocator,tag);
}



KERNEL_PUBLIC config_tag_t* config_tag_load(const void* data,u64 length,const char* password){
	if (length<sizeof(config_file_header_t)){
		return NULL;
	}
	const config_file_header_t* header=data;
	data+=sizeof(config_file_header_t);
	length-=sizeof(config_file_header_t);
	if (header->signature!=CONFIG_FILE_SIGNATURE||header->version!=CONFIG_FILE_VERSION||header->size>length){
		return NULL;
	}
	void* buffer=NULL;
	if (header->flags&COFNIG_FILE_FLAG_ENCRYPTED){
		panic("Decrypt data");
	}
	if (header->flags&COFNIG_FILE_FLAG_COMPRESSED){
		panic("Decompress data");
	}
	config_tag_t* out;
	data=_parse_tag(data,data+length,&out);
	if (buffer){
		pmm_dealloc((u64)(buffer-VMM_HIGHER_HALF_ADDRESS_OFFSET),pmm_align_up_address(header->size)>>PAGE_SIZE_SHIFT,_config_buffer_pmm_counter);
	}
	return (data?out:NULL);
}



KERNEL_PUBLIC config_tag_t* config_tag_load_from_file(vfs_node_t* file,const char* password){
	mmap_region_t* region=mmap_alloc(process_kernel->mmap,0,0,MMAP_REGION_FLAG_NO_WRITEBACK,file);
	config_tag_t* out=config_tag_load((const void*)(region->rb_node.key),region->length,password);
	mmap_dealloc_region(process_kernel->mmap,region);
	return out;
}



KERNEL_PUBLIC _Bool config_tag_save(const config_tag_t* tag,void** data,u64* length,const char* password);



KERNEL_PUBLIC _Bool config_tag_save_to_file(const config_tag_t* tag,vfs_node_t* file,const char* password);
