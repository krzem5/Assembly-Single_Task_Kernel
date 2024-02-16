#if KERNEL_COVERAGE_ENABLED
#include <kernel/elf/elf.h>
#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/serial/serial.h>
#include <kernel/shutdown/shutdown.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "coverage"



#define COVERAGE_SERIAL_PORT (serial_ports+1)

#define COVERAGE_FILE_REPORT_MARKER 0xb8bcbbbe41444347



typedef struct _GCOV_CTR_INFO{
	u32 num;
	s64* values;
} gcov_ctr_info_t;



typedef struct _GCOV_FN_INFO{
	const struct _GCOV_INFO* key;
	u32 ident;
	u32 lineno_checksum;
	u32 cfg_checksum;
	gcov_ctr_info_t ctrs[8];
} gcov_fn_info_t;



typedef struct _GCOV_INFO{
	u32 version;
	u8 _padding[12];
	u32 checksum;
	const char* filename;
	void* merge[8];
	u32 n_functions;
	const gcov_fn_info_t*const* functions;
} gcov_info_t;



static spinlock_t _coverage_lock;



static void KERNEL_NOCOVERAGE _process_gcov_info_section(u64 base,u64 size){
	INFO("Procesing .gcov_info section %p - %p...",base,base+size);
	for (const gcov_info_t*const* info_ptr=(void*)base;(u64)info_ptr<base+size;info_ptr++){
		const gcov_info_t* info=*info_ptr;
		if (!info||!info->merge[0]){
			continue;
		}
		spinlock_acquire_exclusive(&_coverage_lock);
		u64 marker=COVERAGE_FILE_REPORT_MARKER;
		serial_send(COVERAGE_SERIAL_PORT,&marker,sizeof(u64));
		serial_send(COVERAGE_SERIAL_PORT,&(info->version),sizeof(u32));
		serial_send(COVERAGE_SERIAL_PORT,&(info->checksum),sizeof(u32));
		u32 filename_length=0;
		while (info->filename[filename_length]){
			filename_length++;
		}
		serial_send(COVERAGE_SERIAL_PORT,&filename_length,sizeof(u32));
		serial_send(COVERAGE_SERIAL_PORT,info->filename,filename_length);
		u32 function_count=0;
		for (u32 i=0;i<info->n_functions;i++){
			const gcov_fn_info_t* fn_info=info->functions[i];
			function_count+=(fn_info&&fn_info->key==info);
		}
		serial_send(COVERAGE_SERIAL_PORT,&function_count,sizeof(u32));
		for (u32 i=0;i<info->n_functions;i++){
			const gcov_fn_info_t* fn_info=info->functions[i];
			if (!fn_info||fn_info->key!=info){
				continue;
			}
			serial_send(COVERAGE_SERIAL_PORT,&(fn_info->ident),sizeof(u32));
			serial_send(COVERAGE_SERIAL_PORT,&(fn_info->lineno_checksum),sizeof(u32));
			serial_send(COVERAGE_SERIAL_PORT,&(fn_info->cfg_checksum),sizeof(u32));
			serial_send(COVERAGE_SERIAL_PORT,&(fn_info->ctrs->num),sizeof(u32));
			serial_send(COVERAGE_SERIAL_PORT,fn_info->ctrs->values,fn_info->ctrs->num*sizeof(u64));
		}
		spinlock_release_exclusive(&_coverage_lock);
	}
}



static KERNEL_NOCOVERAGE void _listener(void* object,u32 type){
	LOG("Exporting kernel coverage data...");
	u64 size;
	u64 base=kernel_gcov_info_data(&size);
	_process_gcov_info_section(base,size);
	LOG("Exporting module coverage data...");
	HANDLE_FOREACH(module_handle_type){
		module_t* module=handle->object;
		if (module->state==MODULE_STATE_LOADED&&module->gcov_info.size){
			_process_gcov_info_section(module->gcov_info.base,module->gcov_info.size);
		}
	}
}



static notification_listener_t _coverage_shutdown_notification_listener={
	_listener
};



static void _syscall_export_coverage_data(u64 base,u64 size){
	LOG("Exporting user/library coverage data...");
	_process_gcov_info_section(base,size);
}



static syscall_callback_t const _coverage_syscall_functions[]={
	[1]=(syscall_callback_t)_syscall_export_coverage_data
};



_Bool KERNEL_NOCOVERAGE coverage_init(void){
	LOG("Initializing coverage reporting module...");
	INFO("Checking serial port...");
	if (!COVERAGE_SERIAL_PORT->io_port){
		panic("Coverage serial port not present");
	}
	spinlock_init(&_coverage_lock);
	shutdown_register_notification_listener(&_coverage_shutdown_notification_listener);
	syscall_create_table("coverage",_coverage_syscall_functions,sizeof(_coverage_syscall_functions)/sizeof(syscall_callback_t));
	if (IS_ERROR(elf_load("/bin/coverage_test",0,NULL,0,NULL,0))){
		panic("Unable to load coverage tests");
	}
	return 1;
}



#else
#include <kernel/log/log.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "coverage"



_Bool KERNEL_NOCOVERAGE coverage_init(void){
	ERROR("Kernel built without coverage support");
	return 0;
}
#endif
