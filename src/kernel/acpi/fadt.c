#include <kernel/acpi/fadt.h>
#include <kernel/aml/parser.h>
#include <kernel/aml/runtime.h>
#include <kernel/cache/cache.h>
#include <kernel/io/io.h>
#include <kernel/log/log.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "fadt"



// PM1x flags
#define SCI_EN 0x0001
#define SLP_TYP_SHIFT 10
#define SLP_EN 0x2000



typedef struct __attribute__((packed)) _FADT{
	u8 _padding[40];
	u32 dsdt;
	u8 _padding2[2];
	u16 sci_int;
	u32 smi_command_port;
	u8 acpi_enable;
	u8 acpi_disable;
	u8 _padding3[10];
	u32 pm1a_control_block;
	u32 pm1b_control_block;
} fadt_t;



typedef struct __attribute__((packed)) _DSDT{
	u8 _padding[4];
	u32 length;
	u8 _padding2[28];
	u8 data[];
} dsdt_t;



static u32 _fadt_pm1a_control_block;
static u32 _fadt_pm1b_control_block;



void acpi_fadt_load(const void* fadt_ptr){
	LOG("Loading FADT...");
	const fadt_t* fadt=fadt_ptr;
	LOG("Enabling ACPI...");
	if (io_port_in16(fadt->pm1a_control_block)&SCI_EN){
		INFO("ACPI already enabled");
	}
	else{
		io_port_out8(fadt->smi_command_port,fadt->acpi_enable);
		while (!(io_port_in16(fadt->pm1a_control_block)&SCI_EN)){
			__pause();
		}
		if (fadt->pm1b_control_block){
			while (!(io_port_in16(fadt->pm1b_control_block)&SCI_EN)){
				__pause();
			}
		}
	}
	_fadt_pm1a_control_block=fadt->pm1a_control_block;
	_fadt_pm1b_control_block=fadt->pm1b_control_block;
	INFO("Found DSDT at %p",fadt->dsdt);
	const dsdt_t* dsdt=(void*)VMM_TRANSLATE_ADDRESS(fadt->dsdt);
	aml_runtime_init(aml_parse(dsdt->data,dsdt->length-sizeof(dsdt_t)),fadt->sci_int);
}



void KERNEL_NORETURN KERNEL_NOCOVERAGE acpi_fadt_shutdown(_Bool restart){
	asm volatile ("cli":::"memory");
	cache_flush();
	for (u64 i=0;i<0xffff;i++){
		__pause(); // ensure cache flushes properly
	}
	if (restart){
		_acpi_fadt_reboot();
	}
	u16 pm1a_value=(aml_runtime_get_node(NULL,"\\_S5_[0]")->data.integer<<SLP_TYP_SHIFT)|SLP_EN;
	u16 pm1b_value=(aml_runtime_get_node(NULL,"\\_S5_[1]")->data.integer<<SLP_TYP_SHIFT)|SLP_EN;
	io_port_out16(_fadt_pm1a_control_block,pm1a_value);
	if (_fadt_pm1b_control_block){
		io_port_out16(_fadt_pm1b_control_block,pm1b_value);
	}
	for (;;);
}
