#include <coverage/coverage.h>
#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/module/module.h>
#include <kernel/serial/serial.h>
#include <kernel/shutdown/shutdown.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "coverage"



#define COVERAGE_SERIAL_PORT (serial_ports+1)

#define COVERAGE_FILE_REPORT_MARKER 0xb8bcbbbe41444347
#define COVERAGE_FILE_SUCCESS_MARKER 0xb0b4b0b44b4f4b4f



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



static rwlock_t _coverage_lock;
static bool _coverage_failed=0;



static void KERNEL_NOCOVERAGE _process_gcov_info_section(u64 base,u64 size){
	INFO("Procesing .gcov_info section %p - %p...",base,base+size);
	for (const gcov_info_t*const* info_ptr=(void*)base;(u64)info_ptr<base+size;info_ptr++){
		const gcov_info_t* info=*info_ptr;
		if (!info||!info->merge[0]){
			continue;
		}
		rwlock_acquire_write(&_coverage_lock);
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
		rwlock_release_write(&_coverage_lock);
	}
}



static KERNEL_NOCOVERAGE void _syscall_export_coverage_data(u64 base,u64 size){
	LOG("Exporting user coverage data...");
	_process_gcov_info_section(base,size);
}



static KERNEL_NOCOVERAGE void _syscall_process_test_results(u64 pass,u64 fail){
	WARN("%u test%s passed, %u test%s failed",pass,(pass==1?"":"s"),fail,(fail==1?"":"s"));
	if (fail){
		coverage_mark_failure();
	}
}



static KERNEL_NOCOVERAGE void _syscall_shutdown(void){
	if (_coverage_failed){
		return;
	}
	LOG("Exporting kernel coverage data...");
	u64 size;
	u64 base=kernel_gcov_info_data(&size);
	_process_gcov_info_section(base,size);
	LOG("Exporting module coverage data...");
	HANDLE_FOREACH(module_handle_type){
		module_t* module=KERNEL_CONTAINEROF(handle,module_t,handle);
		if (module->state==MODULE_STATE_LOADED&&module->gcov_info_base&&module->gcov_info_size){
			_process_gcov_info_section(module->gcov_info_base,module->gcov_info_size);
		}
	}
	u64 marker=COVERAGE_FILE_SUCCESS_MARKER;
	serial_send(COVERAGE_SERIAL_PORT,&marker,sizeof(u64));
	shutdown(0);
}



static syscall_callback_t const _coverage_syscall_functions[]={
	[1]=(syscall_callback_t)_syscall_export_coverage_data,
	[2]=(syscall_callback_t)_syscall_process_test_results,
	[3]=(syscall_callback_t)_syscall_shutdown,
};



MODULE_PREINIT(){
	LOG("Initializing coverage reporting module...");
	INFO("Checking serial port...");
	if (!COVERAGE_SERIAL_PORT->io_port){
		panic("Coverage serial port not present");
	}
	rwlock_init(&_coverage_lock);
	syscall_create_table("coverage",_coverage_syscall_functions,sizeof(_coverage_syscall_functions)/sizeof(syscall_callback_t));
	return 1;
}



void KERNEL_NOCOVERAGE coverage_mark_failure(void){
	ERROR("Marking coverage as failed");
	_coverage_failed=1;
	shutdown(0);
}
