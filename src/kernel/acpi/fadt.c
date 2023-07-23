#include <kernel/io/io.h>
#include <kernel/log/log.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>



typedef struct __attribute__((packed)) _FADT_GENERIC_ACCESS_STRUCTURE{
	u8 address_space;
	u8 bit_width;
	u8 bit_offset;
	u8 access_size;
	u64 address;
} fadt_generic_access_structure_t;



typedef struct __attribute__((packed)) _FADT{
	u32 _AAA;u32 length;u8 _padding[40-8];
	// u8 _padding[40];
	u32 dsdt;
	u8 _padding2[4];
	u32 smi_command_port;
	u8 acpi_enable;
	u8 acpi_disable;
	u8 _padding3[10];
	u32 pm1a_control_block;
	u32 pm1b_control_block;
	u8 _padding4[116-72];
	fadt_generic_access_structure_t reset_register;
	u8 reset_value;
} fadt_t;



typedef struct __attribute__((packed)) _DSDT{
	u8 _padding[4];
	u32 length;
	u8 _padding2[28];
	u8 data[];
} dsdt_t;



static u32 _fadt_pm1a_control_block;
static u32 _fadt_pm1b_control_block;
static fadt_generic_access_structure_t _fadt_reset_register;
static u8 _fadt_reset_value;
static u16 _dsdt_s5_slp_typa;
static u16 _dsdt_s5_slp_typb;



static void _generic_access_structure_write(const fadt_generic_access_structure_t* gas,u8 value){
	WARN("%u, %p %s",_fadt_reset_register.address_space,_fadt_reset_register.address,&(_fadt_reset_register.address_space));
	ERROR("Unimplemented: _generic_access_structure_write(1)");
	for (;;);
}



void acpi_fadt_load(const void* fadt_ptr){
	LOG("Loading FADT...");
	const fadt_t* fadt=fadt_ptr;
	LOG("Enabling ACPI...");
	if (io_port_in16(fadt->pm1a_control_block)&1){
		INFO("ACPI already enabled");
	}
	else{
		io_port_out8(fadt->smi_command_port,fadt->acpi_enable);
		while (!(io_port_in16(fadt->pm1a_control_block)&1)){
			asm volatile("pause":::"memory");
		}
		if (fadt->pm1b_control_block){
			while (!(io_port_in16(fadt->pm1b_control_block)&1)){
				asm volatile("pause":::"memory");
			}
		}
	}
	INFO("Found DSDT at %p",fadt->dsdt);
	LOG("Parsing DSDT...");
	_fadt_pm1a_control_block=fadt->pm1a_control_block;
	_fadt_pm1b_control_block=fadt->pm1b_control_block;
	_fadt_reset_register=fadt->reset_register;
	WARN("%u\n",fadt->length);
	_fadt_reset_value=fadt->reset_value;
	_dsdt_s5_slp_typa=0;
	_dsdt_s5_slp_typb=0;
	const dsdt_t* dsdt=(void*)VMM_TRANSLATE_ADDRESS(fadt->dsdt);
	for (u32 offset=2;offset<dsdt->length-sizeof(dsdt_t);offset++){
		if ((dsdt->data[offset-1]==0x08||(dsdt->data[offset-2]==0x08&&dsdt->data[offset-1]=='\\'))&&dsdt->data[offset]=='_'&&dsdt->data[offset+1]=='S'&&dsdt->data[offset+2]=='5'&&dsdt->data[offset+3]=='_'&&dsdt->data[offset+4]==0x12){
			INFO("Found \\_S5 object at offset %u",offset);
			offset+=((dsdt->data[offset+5]&0xc0)>>6)+7;
			if (dsdt->data[offset]==0x0a){
				offset++;
			}
			_dsdt_s5_slp_typa=dsdt->data[offset]<<10;
			offset++;
			if (dsdt->data[offset]==0x0a){
				offset++;
			}
			_dsdt_s5_slp_typb=dsdt->data[offset]<<10;
			break;
		}
	}
}



void acpi_fadt_shutdown(_Bool restart){
	if (restart){
		_generic_access_structure_write(&_fadt_reset_register,_fadt_reset_value);
	}
	io_port_out16(_fadt_pm1a_control_block,_dsdt_s5_slp_typa|0x2000);
	if (_fadt_pm1b_control_block){
		io_port_out16(_fadt_pm1b_control_block,_dsdt_s5_slp_typb|0x2000);
	}
	for (;;);
}
