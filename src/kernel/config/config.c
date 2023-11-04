#include <kernel/config/config.h>
#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/mp/process.h>
#include <kernel/types.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "config"



static pmm_counter_descriptor_t _config_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_config");
static pmm_counter_descriptor_t _config_item_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_config_item");
static omm_allocator_t _config_allocator=OMM_ALLOCATOR_INIT_STRUCT("config",sizeof(config_t),8,1,&_config_omm_pmm_counter);
static omm_allocator_t _config_item_allocator=OMM_ALLOCATOR_INIT_STRUCT("config_item",sizeof(config_item_t),8,1,&_config_item_omm_pmm_counter);



config_t* config_load(vfs_node_t* file){
	config_t* out=omm_alloc(&_config_allocator);
	out->head=NULL;
	out->tail=NULL;
	mmap_region_t* region=mmap_alloc(&(process_kernel->mmap),0,0,NULL,MMAP_REGION_FLAG_NO_FILE_WRITEBACK|MMAP_REGION_FLAG_VMM_NOEXECUTE,file);
	const char* buffer=(const char*)(region->rb_node.key);
	for (u64 i=0;i<region->length&&buffer[i];){
		if (buffer[i]=='#'){
			for (i++;i<region->length&&buffer[i]&&buffer[i]!='\n';i++);
			continue;
		}
		if (buffer[i]==' '||buffer[i]=='\t'||buffer[i]=='\r'||buffer[i]=='\n'){
			i++;
			continue;
		}
		u64 j=i;
		for (;j<region->length&&buffer[j]&&buffer[j]!='\n'&&buffer[j]!='=';j++);
		config_item_t* item=omm_alloc(&_config_item_allocator);
		item->prev=out->tail;
		item->next=NULL;
		item->key=smm_alloc(buffer+i,j-i);
		item->value=NULL;
		if (out->tail){
			out->tail->next=item;
		}
		else{
			out->head=item;
		}
		out->tail=item;
		i=j;
		if (buffer[i]!='='){
			continue;
		}
		i++;
		j=i;
		for (;j<region->length&&buffer[j]&&buffer[j]!='\n';j++);
		if (i==j){
			continue;
		}
		item->value=smm_alloc(buffer+i,j-i);
		i=j;
	}
	mmap_dealloc_region(&(process_kernel->mmap),region);
	return out;
}



void config_dealloc(config_t* config){
	for (config_item_t* item=config->head;item;){
		config_item_t* next=item->next;
		smm_dealloc(item->key);
		if (item->value){
			smm_dealloc(item->value);
		}
		omm_dealloc(&_config_item_allocator,item);
		item=next;
	}
	omm_dealloc(&_config_allocator,config);
}
