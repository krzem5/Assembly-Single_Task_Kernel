#include <kernel/acpi/fadt.h>
#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/serial/serial.h>
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



static void KERNEL_NOCOVERAGE _process_gcov_info_section(u64 base,u64 size){
	INFO("Procesing .gcov_info section %p - %p...",base,base+size);
	for (const gcov_info_t*const* info_ptr=(void*)base;(u64)info_ptr<base+size;info_ptr++){
		const gcov_info_t* info=*info_ptr;
		if (!info->merge[0]){
			continue;
		}
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
			serial_send(COVERAGE_SERIAL_PORT,&(fn_info->ident),3*sizeof(u32));
			serial_send(COVERAGE_SERIAL_PORT,&(fn_info->ctrs->num),sizeof(u32));
			serial_send(COVERAGE_SERIAL_PORT,fn_info->ctrs->values,fn_info->ctrs->num*sizeof(u64));
		}
	}
}



void KERNEL_NOCOVERAGE coverage_export(void){
	LOG("Exporting coverage information...");
	INFO("Checking serial port...");
	if (!COVERAGE_SERIAL_PORT->io_port){
		panic("Coverage serial port not present");
	}
	INFO("Writing coverage data...");
	u64 size;
	u64 base=kernel_gcov_info_data(&size);
	_process_gcov_info_section(base,size);
	HANDLE_FOREACH(HANDLE_TYPE_MODULE){
		module_t* module=handle->object;
		if (module->state==MODULE_STATE_LOADED&&module->gcov_info.size){
			_process_gcov_info_section(module->gcov_info.base,module->gcov_info.size);
		}
	}
	acpi_fadt_shutdown(0);
}
