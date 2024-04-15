#include <kernel/config/config.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/mp/process.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kernel/writer/writer.h>
#define KERNEL_LOG_NAME "config"



#define CONFIG_BINARY_FILE_SIGNATURE 0x80474643

#define CONFIG_BINARY_FILE_VERSION 1

#define COFNIG_BINARY_FILE_FLAG_COMPRESSED 1
#define COFNIG_BINARY_FILE_FLAG_ENCRYPTED 2

#define CONFIG_TEXT_FILE_IS_WHITESPACE(c) (!(c)||(c)==' '||(c)=='\t'||(c)=='\n'||(c)=='\r')

#define CONFIG_TEXT_FILE_IS_NUMBER(c) ((c)>='0'&&(c)<='9')

#define CONFIG_TEXT_FILE_IS_IDENTIFIER_START(c) (((c)>='A'&&(c)<='Z')||((c)>='a'&&(c)<='z')||(c)=='_'||(c)=='.'||(c)=='$'||(c)==':')
#define CONFIG_TEXT_FILE_IS_IDENTIFIER(c) (CONFIG_TEXT_FILE_IS_IDENTIFIER_START((c))||CONFIG_TEXT_FILE_IS_NUMBER((c))||(c)=='-')



typedef struct KERNEL_PACKED _CONFIG_BINARY_FILE_HEADER{
	u32 signature;
	u8 version;
	u8 flags;
	u32 size;
} config_file_header_t;



typedef struct KERNEL_PACKED _CONFIG_BINARY_FILE_ENCRYPTION_HEADER{
	u8 hmac[32];
	u8 salt[16];
} config_file_encryption_header_t;



typedef struct KERNEL_PACKED _CONFIG_BINARY_FILE_TAG_HEADER{
	u8 type;
	u8 name_length;
	u16 length;
} config_file_tag_header_t;



static pmm_counter_descriptor_t* KERNEL_INIT_WRITE _config_buffer_pmm_counter=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _config_tag_allocator=NULL;



static const void* _parse_binary_tag(const void* data,const void* end,config_tag_t** out){
	if (data+sizeof(config_file_tag_header_t)>end){
		return NULL;
	}
	const config_file_tag_header_t* header=data;
	data+=sizeof(config_file_tag_header_t);
	if (data+header->name_length+(header->length==0xffff?sizeof(u32):0)>end){
		return NULL;
	}
	*out=omm_alloc(_config_tag_allocator);
	(*out)->parent=NULL;
	(*out)->name=smm_alloc(data,header->name_length);
	(*out)->type=header->type;
	data+=header->name_length;
	u32 tag_length=header->length;
	if (tag_length==0xffff){
		tag_length=*((const u32*)data);
		data+=sizeof(u32);
	}
	if (header->type==CONFIG_TAG_TYPE_ARRAY){
		(*out)->array=amm_alloc(sizeof(config_tag_array_t)+tag_length*sizeof(config_tag_t*));
		(*out)->array->length=tag_length;
		mem_fill((*out)->array->data,sizeof(config_tag_t*)*tag_length,0);
		for (u32 i=0;i<tag_length;i++){
			data=_parse_binary_tag(data,end,(*out)->array->data+i);
			if (!data){
				goto _error;
			}
			(*out)->array->data[i]->parent=*out;
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



static config_tag_t* _parse_binary_config(const void* data,u64 length,const char* password){
	const config_file_header_t* header=data;
	data+=sizeof(config_file_header_t);
	length-=sizeof(config_file_header_t);
	if (header->signature!=CONFIG_BINARY_FILE_SIGNATURE||header->version!=CONFIG_BINARY_FILE_VERSION||header->size>length){
		return NULL;
	}
	if (header->flags&COFNIG_BINARY_FILE_FLAG_ENCRYPTED){
		panic("Decrypt data");
	}
	if (header->flags&COFNIG_BINARY_FILE_FLAG_COMPRESSED){
		panic("Decompress data");
	}
	config_tag_t* out;
	data=_parse_binary_tag(data,data+length,&out);
	return (data?out:NULL);
}



static config_tag_t* _parse_text_config(const char* data,u64 length){
	config_tag_t* out=omm_alloc(_config_tag_allocator);
	out->parent=NULL;
	out->name=smm_alloc("",0);
	out->type=CONFIG_TAG_TYPE_ARRAY;
	out->array=amm_alloc(sizeof(config_tag_array_t));
	out->array->length=0;
	for (const char* end=data+length;data<end;){
		for (;data<end&&CONFIG_TEXT_FILE_IS_WHITESPACE(data[0]);data++);
		if (data>=end){
			break;
		}
		string_t* name=NULL;
		if (CONFIG_TEXT_FILE_IS_IDENTIFIER_START(data[0])){
			u32 name_length=0;
			for (;data+name_length<end&&CONFIG_TEXT_FILE_IS_IDENTIFIER(data[name_length]);name_length++);
			name=smm_alloc(data,name_length);
			data+=name_length;
			for (;data<end&&data[0]!='\n'&&CONFIG_TEXT_FILE_IS_WHITESPACE(data[0]);data++);
		}
		else{
			name=smm_alloc("",0);
		}
		if (data==end||data[0]==','||data[0]=='\n'){
			if (!name->length){
				smm_dealloc(name);
				continue;
			}
			config_tag_t* tag=omm_alloc(_config_tag_allocator);
			tag->name=name;
			tag->type=CONFIG_TAG_TYPE_NONE;
			config_tag_attach(out,tag);
			continue;
		}
		if (name->length||data[0]=='='){
			if (data[0]!='='){
				smm_dealloc(name);
				ERROR("Expected '=', got '%c'",data[0]);
				return NULL;
			}
			data++;
			for (;data<end&&data[0]!='\n'&&CONFIG_TEXT_FILE_IS_WHITESPACE(data[0]);data++);
		}
		if (data==end){
			config_tag_t* tag=omm_alloc(_config_tag_allocator);
			tag->name=name;
			tag->type=CONFIG_TAG_TYPE_NONE;
			config_tag_attach(out,tag);
			continue;
		}
		if (data[0]=='#'){
			for (;data<end&&data[0]!='\n';data++);
			continue;
		}
		if (data[0]=='{'){
			data++;
			config_tag_t* tag=omm_alloc(_config_tag_allocator);
			tag->name=name;
			tag->type=CONFIG_TAG_TYPE_ARRAY;
			tag->array=amm_alloc(sizeof(config_tag_array_t));
			tag->array->length=0;
			config_tag_attach(out,tag);
			out=tag;
			continue;
		}
		if (data[0]=='}'){
			data++;
			out=out->parent;
			if (!out){
				panic("Unbalanced brackets");
			}
			continue;
		}
		if (data[0]=='-'||data[1]=='+'||CONFIG_TEXT_FILE_IS_NUMBER(data[0])){
			panic("Number");
		}
		if (data[0]=='\"'){
			panic("Quoted string");
		}
		u32 string_length=0;
		for (;data+string_length<end&&data[string_length]!='\n';string_length++);
		config_tag_t* tag=omm_alloc(_config_tag_allocator);
		tag->name=name;
		tag->type=CONFIG_TAG_TYPE_STRING;
		tag->string=smm_alloc(data,string_length);
		data+=string_length;
		config_tag_attach(out,tag);
	}
	return out;
}



static void _save_binary_tag(writer_t* writer,const config_tag_t* tag){
	u32 tag_type=tag->type;
	u32 tag_length=0;
	if (tag->type==CONFIG_TAG_TYPE_ARRAY){
		tag_length=tag->array->length;
	}
	else if (tag->type==CONFIG_TAG_TYPE_STRING){
		tag_length=tag->string->length;
	}
	else if (tag->type==CONFIG_TAG_TYPE_INT){
		tag_type=(tag->int_<0?CONFIG_TAG_TYPE_INT_NEGATIVE:CONFIG_TAG_TYPE_INT);
		tag_length=(tag->int_?(64-__builtin_ctzll((tag->int_<0?-tag->int_:tag->int_))+7)>>3:0);
	}
	config_file_tag_header_t header={
		tag_type,
		tag->name->length,
		(tag_length<0xffff?tag_length:0xffff)
	};
	writer_append(writer,&header,sizeof(config_file_tag_header_t));
	writer_append(writer,tag->name->data,tag->name->length);
	if (tag_length>=0xffff){
		writer_append_u32(writer,tag_length);
	}
	if (tag->type==CONFIG_TAG_TYPE_ARRAY){
		for (u32 i=0;i<tag->array->length;i++){
			_save_binary_tag(writer,tag->array->data[i]);
		}
	}
	else if (tag->type==CONFIG_TAG_TYPE_STRING){
		writer_append(writer,tag->string->data,tag->string->length);
	}
	else if (tag->type==CONFIG_TAG_TYPE_INT){
		u64 value=(tag->int_<0?-tag->int_:tag->int_);
		for (u32 i=0;i<tag_length;i++){
			writer_append_u8(writer,value>>(i>>3));
		}
	}
}



KERNEL_INIT(){
	LOG("Initializing config tags...");
	_config_buffer_pmm_counter=pmm_alloc_counter("config_buffer");
	_config_tag_allocator=omm_init("config_tag",sizeof(config_tag_t),8,4,pmm_alloc_counter("omm_config_tag"));
	spinlock_init(&(_config_tag_allocator->lock));
}



KERNEL_PUBLIC config_tag_t* config_tag_create(u32 type,const char* name){
	config_tag_t* out=omm_alloc(_config_tag_allocator);
	out->parent=NULL;
	out->name=smm_alloc(name,0);
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



KERNEL_PUBLIC void config_tag_attach(config_tag_t* tag,config_tag_t* child){
	if (tag->type!=CONFIG_TAG_TYPE_ARRAY){
		panic("config_tag_attach: tag is not an array");
	}
	tag->array->length++;
	tag->array=amm_realloc(tag->array,sizeof(config_tag_array_t)+tag->array->length*sizeof(config_tag_t*));
	tag->array->data[tag->array->length-1]=child;
	child->parent=tag;
}



KERNEL_PUBLIC void config_tag_detach(config_tag_t* child){
	if (!child->parent){
		return;
	}
	panic("config_tag_detach");
}



KERNEL_PUBLIC config_tag_t* config_load(const void* data,u64 length,const char* password){
	if (length>=sizeof(u32)&&*((const u32*)data)==CONFIG_BINARY_FILE_SIGNATURE){
		return _parse_binary_config(data,length,password);
	}
	return _parse_text_config(data,length);
}



KERNEL_PUBLIC config_tag_t* config_load_from_file(vfs_node_t* file,const char* password){
	mmap_region_t* region=mmap_alloc(process_kernel->mmap,0,0,MMAP_REGION_FLAG_NO_WRITEBACK,file);
	config_tag_t* out=config_load((const void*)(region->rb_node.key),region->length,password);
	mmap_dealloc_region(process_kernel->mmap,region);
	return out;
}



KERNEL_PUBLIC _Bool config_save(const config_tag_t* tag,void** data,u64* length,const char* password);



KERNEL_PUBLIC _Bool config_save_to_file(const config_tag_t* tag,vfs_node_t* file,const char* password){
	writer_t* writer=writer_init(file,NULL);
	config_file_header_t header={
		CONFIG_BINARY_FILE_SIGNATURE,
		CONFIG_BINARY_FILE_VERSION,
		0,
		0xffffffff
	};
	writer_append(writer,&header,sizeof(config_file_header_t));
	if (!password){
		_save_binary_tag(writer,tag);
	}
	else{
		panic("config_save_to_file: password encryption");
	}
	header.size=writer_deinit(writer);
	vfs_node_write(file,0,&header,sizeof(config_file_header_t),0);
	return 1;
}
