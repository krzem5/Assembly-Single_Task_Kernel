#if KERNEL_COVERAGE_ENABLED
#include <kernel/acpi/fadt.h>
#include <kernel/io/io.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "coverage"



#define GCOV_COUNTER_V_TOPN 3
#define GCOV_COUNTER_V_INDIR 4
#define GCOV_COUNTER_COUNT 8



typedef struct _GCOV_CTR_INFO{
	u32 num;
	s64* values;
} gcov_ctr_info_t;



typedef struct _GCOV_FN_INFO{
	const struct _GCOV_INFO* key;
	u32 ident;
	u32 lineno_checksum;
	u32 cfg_checksum;
	gcov_ctr_info_t ctrs[];
} gcov_fn_info_t;



typedef struct _GCOV_INFO{
	u32 version;
	u8 _padding[8];
	u32 stamp;
	u32 checksum;
	const char* filename;
	void* merge[GCOV_COUNTER_COUNT];
	u32 n_functions;
	const gcov_fn_info_t*const* functions;
} gcov_info_t;



static void KERNEL_NOCOVERAGE _output_bytes(const void* buffer,u32 length){
	for (;length;length--){
		while (!(io_port_in8(0x2fd)&0x20)){
			__pause();
		}
		io_port_out8(0x2f8,*((const u8*)buffer));
		buffer++;
	}
}



static void KERNEL_NOCOVERAGE _output_int(u32 value){
	_output_bytes(&value,sizeof(u32));
}



static void KERNEL_NOCOVERAGE _output_counter(s64 counter){
	_output_int(counter);
	_output_int(counter>>32);
}



static void KERNEL_NOCOVERAGE _output_string(const char* str){
	u32 length=0;
	do{
		length++;
	} while (str[length-1]);
	_output_int(length);
	_output_bytes(str,length);
}



extern u64 __KERNEL_GCOV_INFO_START__[];
extern u64 __KERNEL_GCOV_INFO_END__[];



void KERNEL_CORE_CODE KERNEL_NOCOVERAGE __gcov_merge_add(void){
	return;
}



void KERNEL_NORETURN KERNEL_NOCOVERAGE syscall_dump_coverage_data(syscall_registers_t* regs){
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
	for (const gcov_info_t*const* info_ptr=(void*)(&__KERNEL_GCOV_INFO_START__);(void*)info_ptr<(void*)(&__KERNEL_GCOV_INFO_END__);info_ptr++){
		const gcov_info_t* info=*info_ptr;
		_output_string(info->filename);
		_output_bytes("adcg",4);
		_output_int(info->version);
		_output_int(info->stamp);
		_output_int(info->checksum);
		for (u32 i=0;i<info->n_functions;i++){
			const gcov_fn_info_t* gfi_ptr=info->functions[i];
			u32 length=(gfi_ptr&&gfi_ptr->key==info?3*sizeof(u32):0);
			_output_int(0x01000000);
			_output_int(length);
			if (!length){
				continue;
			}
			_output_int(gfi_ptr->ident);
			_output_int(gfi_ptr->lineno_checksum);
			_output_int(gfi_ptr->cfg_checksum);
			for (u32 j=0;j<GCOV_COUNTER_COUNT;j++){
				if (!info->merge[j]){
					continue;
				}
				const gcov_ctr_info_t* ci_ptr=gfi_ptr->ctrs+j;
				if (j==GCOV_COUNTER_V_TOPN||j==GCOV_COUNTER_V_INDIR){
					ERROR("Unimplemented");
					continue;
				}
				_output_int(0x01a10000+(j<<17));
				for (u32 k=0;k<ci_ptr->num;k++){
					if (ci_ptr->values[k]){
						goto _counter_data_exists;
					}
				}
				_output_int(-ci_ptr->num*sizeof(u64));
				continue;
_counter_data_exists:
				_output_int(ci_ptr->num*sizeof(u64));
				for (u32 k=0;k<ci_ptr->num;k++){
					_output_counter(ci_ptr->values[k]);
				}
			}
		}
		_output_int(0);
	}
	acpi_fadt_shutdown(0);
}



#endif
