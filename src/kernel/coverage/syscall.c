#if KERNEL_COVERAGE_ENABLED
#include <kernel/acpi/fadt.h>
#include <kernel/io/io.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "coverage"



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



extern const gcov_info_t* __KERNEL_SECTION_gcov_info_START__;
extern const gcov_info_t* __KERNEL_SECTION_gcov_info_END__;



static void KERNEL_NOCOVERAGE _output_bytes(const void* buffer,u32 length){
	for (;length;length--){
		SPINLOOP(!(io_port_in8(0x2fd)&0x20));
		io_port_out8(0x2f8,*((const u8*)buffer));
		buffer++;
	}
}



static KERNEL_INLINE void KERNEL_NOCOVERAGE _output_int(u32 value){
	_output_bytes(&value,sizeof(u32));
}



void KERNEL_CORE_CODE KERNEL_NOCOVERAGE __gcov_merge_add(void){
	return;
}



void KERNEL_NORETURN KERNEL_NOCOVERAGE syscall_coverage_dump_data(syscall_registers_t* regs){
	LOG("Dumping coverage information...");
	INFO("Initializing serial port...");
	io_port_out8(0x2f9,0x00);
	io_port_out8(0x2fb,0x80);
	io_port_out8(0x2f8,0x03);
	io_port_out8(0x2f9,0x00);
	io_port_out8(0x2fb,0x03);
	io_port_out8(0x2fa,0xc7);
	io_port_out8(0x2fc,0x03);
	INFO("Writing coverage data...");
	for (const gcov_info_t*const* info_ptr=&__KERNEL_SECTION_gcov_info_START__;info_ptr<&__KERNEL_SECTION_gcov_info_END__;info_ptr++){
		const gcov_info_t* info=*info_ptr;
		if (!info->merge[0]){
			continue;
		}
		_output_int(info->version);
		_output_int(info->checksum);
		u32 filename_length=0;
		while (info->filename[filename_length]){
			filename_length++;
		}
		_output_int(filename_length);
		_output_bytes(info->filename,filename_length);
		u32 function_count=0;
		for (u32 i=0;i<info->n_functions;i++){
			const gcov_fn_info_t* fn_info=info->functions[i];
			function_count+=(fn_info&&fn_info->key==info);
		}
		_output_int(function_count);
		for (u32 i=0;i<info->n_functions;i++){
			const gcov_fn_info_t* fn_info=info->functions[i];
			if (!fn_info||fn_info->key!=info){
				continue;
			}
			_output_int(fn_info->ident);
			_output_int(fn_info->lineno_checksum);
			_output_int(fn_info->cfg_checksum);
			_output_int(fn_info->ctrs->num);
			_output_bytes(fn_info->ctrs->values,fn_info->ctrs->num*sizeof(u64));
		}
	}
	acpi_fadt_shutdown(0);
}



#else
#include <kernel/syscall/syscall.h>



void syscall_coverage_dump_data(syscall_registers_t* regs){
	return;
}



#endif
