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



#endif
