#include <kernel/config/config.h>
#include <kernel/fs/fs.h>
#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/lock/mutex.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/smm.h>
#include <kernel/module/module.h>
#include <kernel/mp/event.h>
#include <kernel/mp/thread.h>
#include <kernel/notification/notification.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/string.h>
#include <kernel/util/util.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "fs_loader"



#define FS_LIST_FILE "/etc/fs_list.config"



#define LIST_ENTRY_FLAG_MATCH_UUID 1
#define LIST_ENTRY_FLAG_MATCH_GUID 2
#define LIST_ENTRY_FLAG_MOUNTED 4
#define LIST_ENTRY_FLAG_BARRIER 8
#define LIST_ENTRY_FLAG_FOUND 16
#define LIST_ENTRY_FLAG_REQUIRED 32



typedef struct _LIST_ENTRY{
	struct _LIST_ENTRY* next;
	char* path;
	char* type;
	u8 uuid[16];
	u8 guid[16];
	u32 flags;
	handle_id_t fs_handle;
} list_entry_t;



typedef struct _LIST{
	mutex_t* lock;
	event_t* ready_event;
	u32 unmatched_count;
	list_entry_t* head;
} list_t;



static omm_allocator_t* KERNEL_INIT_WRITE _fs_loader_list_entry_allocator=NULL;
static list_t _fs_loader_list;



static void _dealloc_entry(list_entry_t* entry){
	amm_dealloc(entry->path);
	if (entry->type){
		amm_dealloc(entry->type);
	}
	omm_dealloc(_fs_loader_list_entry_allocator,entry);
}



static char* _duplicate_string(const char* str){
	u32 length=smm_length(str)+1;
	char* out=amm_alloc(length);
	mem_copy(out,str,length);
	return out;
}



static bool _parse_uuid(const char* uuid,u8* out){
	if (smm_length(uuid)!=36){
		return 0;
	}
	u32 j=0;
	for (u32 i=0;i<36;i++){
		if (0x842100&(1ull<<i)){
			if (uuid[i]!='-'){
				return 0;
			}
			continue;
		}
		if (!(j&1)){
			out[j>>1]=0;
		}
		out[j>>1]<<=4;
		if ('0'<=uuid[i]&&uuid[i]<='9'){
			out[j>>1]|=uuid[i]-'0';
		}
		else if ('A'<=uuid[i]&&uuid[i]<='Z'){
			out[j>>1]|=uuid[i]-'A'+10;
		}
		else if ('a'<=uuid[i]&&uuid[i]<='z'){
			out[j>>1]|=uuid[i]-'a'+10;
		}
		else{
			return 0;
		}
		j++;
	}
	return 1;
}



static KERNEL_AWAITS void _match_filesystem(const filesystem_t* fs){
	if (!fs->is_ready){
		return;
	}
	mutex_acquire(_fs_loader_list.lock);
	bool after_barrier=0;
	for (list_entry_t* entry=_fs_loader_list.head;entry;entry=entry->next){
		if (entry->flags&LIST_ENTRY_FLAG_FOUND){
			continue;
		}
		if (entry->flags&LIST_ENTRY_FLAG_MATCH_UUID){
			for (u32 i=0;i<16;i++){
				if (fs->uuid[i]!=entry->uuid[i]){
					goto _match_next_entry;
				}
			}
		}
		if (entry->flags&LIST_ENTRY_FLAG_MATCH_GUID){
			if (!fs->partition){
				goto _match_next_entry;
			}
			for (u32 i=0;i<16;i++){
				if (fs->partition->guid[i]!=entry->guid[i]){
					goto _match_next_entry;
				}
			}
		}
		if (entry->type&&!str_equal(fs->descriptor->config->name,entry->type)){
			goto _match_next_entry;
		}
		entry->flags|=LIST_ENTRY_FLAG_FOUND;
		entry->fs_handle=fs->handle.rb_node.key;
		if (after_barrier){
			break;
		}
		bool was_fs_ready=!_fs_loader_list.unmatched_count;
		for (;entry;entry=entry->next){
			if ((entry->flags&(LIST_ENTRY_FLAG_FOUND|LIST_ENTRY_FLAG_MOUNTED))!=LIST_ENTRY_FLAG_FOUND){
				continue;
			}
			handle_t* fs_handle=handle_lookup_and_acquire(entry->fs_handle,fs_handle_type);
			if (!fs_handle){
				entry->flags&=~LIST_ENTRY_FLAG_FOUND;
				_fs_loader_list.unmatched_count+=!!(entry->flags&LIST_ENTRY_FLAG_REQUIRED);
				continue;
			}
			vfs_mount(KERNEL_CONTAINEROF(fs_handle,filesystem_t,handle),(str_equal(entry->path,"/")?NULL:entry->path),0);
			handle_release(fs_handle);
			entry->flags|=LIST_ENTRY_FLAG_MOUNTED;
			_fs_loader_list.unmatched_count-=!!(entry->flags&LIST_ENTRY_FLAG_REQUIRED);
		}
		if (was_fs_ready){
			if (_fs_loader_list.unmatched_count){
				event_set_active(_fs_loader_list.ready_event,0,1);
			}
			break;
		}
		if (!_fs_loader_list.unmatched_count){
			LOG("All required conditions fulfilled");
			event_dispatch(_fs_loader_list.ready_event,EVENT_DISPATCH_FLAG_DISPATCH_ALL|EVENT_DISPATCH_FLAG_SET_ACTIVE|EVENT_DISPATCH_FLAG_BYPASS_ACL);
		}
		break;
_match_next_entry:
		if (entry->flags&LIST_ENTRY_FLAG_BARRIER){
			after_barrier=1;
		}
	}
	mutex_release(_fs_loader_list.lock);
}



static KERNEL_AWAITS bool _reload_config_file(void){
	vfs_node_t* file=vfs_lookup(NULL,FS_LIST_FILE,0,0,0);
	if (!file){
		ERROR("Unable to locate filesystem list file");
		return 0;
	}
	config_tag_t* root_tag=config_load_from_file(file,NULL);
	vfs_node_unref(file);
	list_entry_t* list_head=NULL;
	list_entry_t* list_tail=NULL;
	u32 unmatched_count=0;
	for (config_tag_t* fs_tag=config_tag_iter_start(root_tag);fs_tag;fs_tag=config_tag_iter_next(root_tag,fs_tag)){
		if (fs_tag->type!=CONFIG_TAG_TYPE_ARRAY){
			continue;
		}
		const char* path=NULL;
		const char* uuid=NULL;
		const char* guid=NULL;
		const char* type=NULL;
		bool required=1;
		bool barrier=0;
		config_tag_t* tag=NULL;
		if (config_tag_find(fs_tag,"path",0,&tag)&&tag->type==CONFIG_TAG_TYPE_STRING){
			path=tag->string->data;
		}
		if (config_tag_find(fs_tag,"uuid",0,&tag)&&tag->type==CONFIG_TAG_TYPE_STRING){
			uuid=tag->string->data;
		}
		if (config_tag_find(fs_tag,"guid",0,&tag)&&tag->type==CONFIG_TAG_TYPE_STRING){
			guid=tag->string->data;
		}
		if (config_tag_find(fs_tag,"type",0,&tag)&&tag->type==CONFIG_TAG_TYPE_STRING){
			type=tag->string->data;
		}
		if (config_tag_find(fs_tag,"not-required",0,&tag)){
			required=0;
		}
		if (config_tag_find(fs_tag,"barrier",0,&tag)){
			barrier=1;
		}
		if (!path){
			ERROR("'path' tag missing");
			continue;
		}
		list_entry_t* entry=omm_alloc(_fs_loader_list_entry_allocator);
		entry->path=_duplicate_string(path);
		entry->type=(type?_duplicate_string(type):NULL);
		entry->flags=(uuid?LIST_ENTRY_FLAG_MATCH_UUID:0)|(guid?LIST_ENTRY_FLAG_MATCH_GUID:0)|(barrier?LIST_ENTRY_FLAG_BARRIER:0)|(required?LIST_ENTRY_FLAG_REQUIRED:0);
		if (uuid&&str_equal(uuid,"boot")){
			mem_copy(entry->uuid,kernel_get_boot_uuid(),16);
		}
		else if (uuid&&!_parse_uuid(uuid,entry->uuid)){
			ERROR("Invalid UUID: '%s'",uuid);
			_dealloc_entry(entry);
			continue;
		}
		if (guid&&!_parse_uuid(guid,entry->guid)){
			ERROR("Invalid GUID: '%s'",guid);
			_dealloc_entry(entry);
			continue;
		}
		if (list_tail){
			list_tail->next=entry;
		}
		else{
			list_head=entry;
		}
		list_tail=entry;
		entry->next=NULL;
		unmatched_count+=required;
	}
	config_tag_delete(root_tag);
	if (!list_head){
		return 0;
	}
	mutex_acquire(_fs_loader_list.lock);
	list_entry_t* old_head=_fs_loader_list.head;
	_fs_loader_list.unmatched_count=unmatched_count;
	_fs_loader_list.head=list_head;
	mutex_release(_fs_loader_list.lock);
	while (old_head){
		list_entry_t* next=old_head->next;
		_dealloc_entry(old_head);
		old_head=next;
	}
	HANDLE_FOREACH(fs_handle_type){
		_match_filesystem(KERNEL_CONTAINEROF(handle,filesystem_t,handle));
	}
	return 1;
}



static KERNEL_AWAITS void _update_notification_thread(void){
	notification_consumer_t* consumer=notification_consumer_create(fs_notification_dispatcher);
	while (1){
		notification_t notification;
		if (!notification_consumer_get(consumer,1,&notification)){
			continue;
		}
		if (notification.type==FS_CREATE_NOTIFICATION&&notification.length==sizeof(filesystem_create_notification_data_t)){
			const filesystem_create_notification_data_t* data=notification.data;
			handle_t* handle=handle_lookup_and_acquire(data->fs_handle,fs_handle_type);
			if (!handle){
				continue;
			}
			_match_filesystem(KERNEL_CONTAINEROF(handle,const filesystem_t,handle));
			handle_release(handle);
		}
	}
}



MODULE_INIT(){
	LOG("Mounting filesystems...");
	_fs_loader_list_entry_allocator=omm_init("fs_loader.list.entry",sizeof(list_entry_t),8,4);
	rwlock_init(&(_fs_loader_list_entry_allocator->lock));
	_fs_loader_list.lock=mutex_create("fs_loader.list");
	_fs_loader_list.ready_event=event_create("fs_loader.ready",NULL);
	_fs_loader_list.unmatched_count=0;
	_fs_loader_list.head=NULL;
	if (!_reload_config_file()){
		panic("Root filesystem not found");
	}
	thread_create_kernel_thread(NULL,"fs_loader.update",_update_notification_thread,0);
	event_await(&(_fs_loader_list.ready_event),1,0);
}



MODULE_DECLARE(0);
