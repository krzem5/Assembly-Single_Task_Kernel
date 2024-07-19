#include <kernel/format/format.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/memory/vmm.h>
#include <kernel/serial/serial.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>



#define MAX_NAME_LENGTH 31
#define MAX_DATA_SIZE 256

#define LOG_EARLY_BUFFER_SIZE 16384 // 16 kB
#define LOG_BUFFER_SIZE 0x400000 // 4 MB



static pmm_counter_descriptor_t* KERNEL_INIT_WRITE _log_pmm_counter=NULL;
static u8 KERNEL_EARLY_WRITE _log_early_buffer_data[LOG_EARLY_BUFFER_SIZE];
static log_buffer_t KERNEL_EARLY_WRITE _log_early_buffer={
	NULL,
	NULL,
	0,
	LOG_EARLY_BUFFER_SIZE,
	_log_early_buffer_data
};
static rwlock_t _log_buffer_lock=RWLOCK_INIT_STRUCT;
static log_buffer_t* _log_buffer_head=NULL;
static log_buffer_t* _log_buffer_tail=&_log_early_buffer;
#ifdef KERNEL_RELEASE
static u32 _log_mask=(1<<LOG_TYPE_INFO)|(1<<LOG_TYPE_LOG);
#else
static u32 _log_mask=0;
#endif



static void _alloc_new_buffer(void){
	void* data=(void*)(pmm_alloc(pmm_align_up_address(LOG_BUFFER_SIZE)>>PAGE_SIZE_SHIFT,_log_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	log_buffer_t* out=data;
	out->prev=_log_buffer_tail;
	out->next=NULL;
	out->offset=0;
	out->size=pmm_align_up_address(LOG_BUFFER_SIZE)-sizeof(log_buffer_t);
	out->ptr=data+sizeof(log_buffer_t);
	_log_buffer_tail->next=out;
	_log_buffer_tail=out;
}



KERNEL_EARLY_EARLY_INIT(){
	rwlock_init(&_log_buffer_lock);
	_log_pmm_counter=pmm_alloc_counter("kernel.log");
	_alloc_new_buffer();
	_log_buffer_head=_log_buffer_tail;
	_log_buffer_tail->prev=NULL;
	_log_buffer_tail->offset=_log_early_buffer.offset;
	mem_copy(_log_buffer_tail->ptr,_log_early_buffer.ptr,_log_early_buffer.offset);
}



KERNEL_PUBLIC void log(u32 type,const char* name,const char* template,...){
	char data[MAX_DATA_SIZE];
	__builtin_va_list va;
	__builtin_va_start(va,template);
	u32 data_length=format_string_va(data,MAX_DATA_SIZE,template,&va);
	__builtin_va_end(va);
	u32 name_length=smm_length(name);
	if (name_length>MAX_NAME_LENGTH){
		name_length=MAX_NAME_LENGTH;
	}
	u16 log_entry_size=sizeof(log_entry_t)+name_length+1+data_length+1;
	rwlock_acquire_write(&_log_buffer_lock);
	if (_log_buffer_tail->offset+log_entry_size>_log_buffer_tail->size){
		if (!_log_pmm_counter){
			rwlock_release_write(&_log_buffer_lock);
			goto _skip_log_entry;
		}
		_alloc_new_buffer();
	}
	log_entry_t* entry=_log_buffer_tail->ptr+_log_buffer_tail->offset;
	entry->type=LOG_TYPE_INVALID;
	_log_buffer_tail->offset+=log_entry_size;
	rwlock_release_write(&_log_buffer_lock);
	entry->name_length=name_length;
	entry->data_length=data_length;
	mem_copy(entry->name_and_data,name,name_length);
	entry->name_and_data[name_length]=0;
	mem_copy(entry->name_and_data+name_length+1,data,data_length);
	entry->name_and_data[name_length+1+data_length]=0;
	entry->type=type;
_skip_log_entry:
	if (_log_mask&(1<<type)){
		return;
	}
	char buffer[MAX_DATA_SIZE];
	const char* format_code="0";
	if (type==LOG_TYPE_INFO){
		format_code="37";
	}
	else if (type==LOG_TYPE_LOG){
		format_code="1;97";
	}
	else if (type==LOG_TYPE_WARN){
		format_code="93";
	}
	else if (type==LOG_TYPE_ERROR){
		format_code="1;91";
	}
	serial_send(serial_default_port,buffer,format_string(buffer,MAX_DATA_SIZE,"\x1b[90m[%s] \x1b[%sm%s\x1b[0m\n",name,format_code,data));
}



KERNEL_PUBLIC void log_direct(const char* template,...){
	char buffer[MAX_DATA_SIZE];
	__builtin_va_list va;
	__builtin_va_start(va,template);
	u32 offset=format_string_va(buffer,MAX_DATA_SIZE,template,&va);
	__builtin_va_end(va);
	serial_send(serial_default_port,buffer,offset);
}



KERNEL_PUBLIC void log_mask_type(u32 type){
	_log_mask|=1<<type;
}



KERNEL_PUBLIC void log_unmask_type(u32 type){
	_log_mask&=~(1<<type);
}



KERNEL_PUBLIC const log_buffer_t* log_get_buffer(void){
	return _log_buffer_head;
}
