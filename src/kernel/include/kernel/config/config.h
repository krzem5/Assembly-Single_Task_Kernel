#ifndef _KERNEL_CONFIG_CONFIG_H_
#define _KERNEL_CONFIG_CONFIG_H_ 1
#include <kernel/memory/smm.h>
#include <kernel/types.h>
#include <kernel/vfs/node.h>



typedef struct _CONFIG_ITEM{
	struct _CONFIG_ITEM* prev;
	struct _CONFIG_ITEM* next;
	string_t* key;
	string_t* value;
} config_item_t;



typedef struct _CONFIG{
	config_item_t* head;
	config_item_t* tail;
} config_t;



config_t* config_load(vfs_node_t* file);



void config_dealloc(config_t* config);



/************************************************/



#define CONFIG_TAG_TYPE_NONE 0
#define CONFIG_TAG_TYPE_ARRAY 1
#define CONFIG_TAG_TYPE_STRING 2
#define CONFIG_TAG_TYPE_INT 3
#define CONFIG_TAG_TYPE_INT_NEGATIVE 4



typedef struct _CONFIG_TAG{
	struct _CONFIG_TAG* parent;
	string_t* name;
	u32 type;
	union{
		struct _CONFIG_TAG_ARRAY* array;
		string_t* string;
		s64 int_;
	};
} config_tag_t;



typedef struct _CONFIG_TAG_ARRAY{
	u32 length;
	config_tag_t* data[];
} config_tag_array_t;



config_tag_t* config_tag_create(u32 type,const char* name,u8 name_length);



void config_tag_delete(config_tag_t* tag);



void config_tag_attach(config_tag_t* tag,config_tag_t* child);



void config_tag_detach(config_tag_t* child);



config_tag_t* config_tag_load(const void* data,u64 length,const char* password);



config_tag_t* config_tag_load_from_file(vfs_node_t* file,const char* password);



_Bool config_tag_save(const config_tag_t* tag,void** data,u64* length,const char* password);



_Bool config_tag_save_to_file(const config_tag_t* tag,vfs_node_t* file,const char* password);



#endif
