#include <kernel/bios/bios.h>
#include <kernel/format/format.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/smm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "bios"



#define SMBIOS_HEADER_TYPE_BIOS_INFORMATION 0
#define SMBIOS_HEADER_TYPE_SYSTEM_INFORMATION 1
#define SMBIOS_HEADER_TYPE_BASEBOARD_INFORMATION 2



typedef struct KERNEL_PACKED _SMBIOS{
	u32 entry_point_string;
	u8 _padding[2];
	u8 major_version;
	u8 minor_version;
	u8 _padding2[14];
	u16 table_length;
	u32 table_address;
} smbios_t;



typedef struct KERNEL_PACKED _SMBIO_HEADER{
	u8 type;
	u8 length;
	u8 _padding[2];
	union{
		struct KERNEL_PACKED{
			u8 vendor;
			u8 bios_version;
		} bios_information;
		struct KERNEL_PACKED{
			u8 manufacturer;
			u8 product_name;
			u8 version;
			u8 serial_number;
			u8 uuid[16];
			u8 wakeup_type;
		} system_information;
		struct KERNEL_PACKED{
			u8 _padding[3];
			u8 serial_number;
		} baseboard_information;
	};
} smbios_header_t;



static const char*const KERNEL_EARLY_READ _bios_wakeup_type_to_string[]={
	[BIOS_DATA_WAKEUP_TYPE_UNKNOWN]="Unknown",
	[BIOS_DATA_WAKEUP_TYPE_POWER_SWITCH]="Power switch",
	[BIOS_DATA_WAKEUP_TYPE_AC_POWER]="AC power",
};

KERNEL_PUBLIC bios_data_t KERNEL_INIT_WRITE bios_data;



static u64 KERNEL_EARLY_EXEC _get_header_length(const smbios_header_t* header){
	const u8* ptr=((const u8*)header)+header->length;
	while (ptr[0]||ptr[1]){
		ptr++;
	}
	return (u64)(ptr-((const u8*)header)+2);
}



static const char* KERNEL_EARLY_EXEC _get_header_string(const smbios_header_t* header,u8 index){
	if (!index){
		return "";
	}
	const char* ptr=((const char*)header)+header->length;
	while (index>1){
		index--;
		while (*ptr){
			ptr++;
		}
		ptr++;
	}
	return ptr;
}



KERNEL_INIT(){
	LOG("Loading BIOS data...");
	const smbios_t* smbios=(void*)vmm_identity_map(kernel_data.smbios_address,sizeof(smbios_t));
	INFO("Found SMBIOS at %p (revision %u.%u)",((u64)smbios)-VMM_HIGHER_HALF_ADDRESS_OFFSET,smbios->major_version,smbios->minor_version);
	INFO("SMBIOS table: %p - %p",smbios->table_address,smbios->table_address+smbios->table_length);
	u64 table_start=vmm_identity_map(smbios->table_address,smbios->table_length);
	u64 table_end=table_start+smbios->table_length;
	_Bool serial_number_found=0;
	bios_data.wakeup_type=BIOS_DATA_WAKEUP_TYPE_UNKNOWN;
	for (u64 offset=table_start;offset<table_end;){
		const smbios_header_t* header=(void*)offset;
		switch (header->type){
			case SMBIOS_HEADER_TYPE_BIOS_INFORMATION:
				bios_data.bios_vendor=smm_alloc(_get_header_string(header,header->bios_information.vendor),0);
				bios_data.bios_version=smm_alloc(_get_header_string(header,header->bios_information.bios_version),0);
				break;
			case SMBIOS_HEADER_TYPE_SYSTEM_INFORMATION:
				bios_data.manufacturer=smm_alloc(_get_header_string(header,header->system_information.manufacturer),0);
				bios_data.product=smm_alloc(_get_header_string(header,header->system_information.product_name),0);
				bios_data.version=smm_alloc(_get_header_string(header,header->system_information.version),0);
				if (!serial_number_found){
					bios_data.serial_number=smm_alloc(_get_header_string(header,header->system_information.serial_number),0);
				}
				for (u8 i=0;i<4;i++){
					bios_data.uuid[i]=header->system_information.uuid[3-i];
				}
				for (u8 i=0;i<2;i++){
					bios_data.uuid[i+4]=header->system_information.uuid[5-i];
					bios_data.uuid[i+6]=header->system_information.uuid[7-i];
				}
				for (u8 i=8;i<16;i++){
					bios_data.uuid[i]=header->system_information.uuid[i];
				}
				switch (header->system_information.wakeup_type){
					case 6:
						bios_data.wakeup_type=BIOS_DATA_WAKEUP_TYPE_POWER_SWITCH;
						break;
					case 8:
						bios_data.wakeup_type=BIOS_DATA_WAKEUP_TYPE_AC_POWER;
						break;
				}
				break;
			case SMBIOS_HEADER_TYPE_BASEBOARD_INFORMATION:
				bios_data.serial_number=smm_alloc(_get_header_string(header,header->baseboard_information.serial_number),0);
				serial_number_found=1;
				break;
		}
		offset+=_get_header_length(header);
	}
	char buffer[37];
	format_string(buffer,37,"%g",bios_data.uuid);
	bios_data.uuid_str=smm_alloc(buffer,36);
	bios_data.wakeup_type_str=smm_alloc(_bios_wakeup_type_to_string[bios_data.wakeup_type],0);
	INFO("BIOS data:");
	INFO("  BIOS vendor: %s",bios_data.bios_vendor->data);
	INFO("  BIOS version: %s",bios_data.bios_version->data);
	INFO("  Manufacturer: %s",bios_data.manufacturer->data);
	INFO("  Product: %s",bios_data.product->data);
	INFO("  Version: %s",bios_data.version->data);
	INFO("  Serial number: %s",bios_data.serial_number->data);
	INFO("  UUID: %s",bios_data.uuid_str->data);
	INFO("  Last wakeup: %s",bios_data.wakeup_type_str->data);
}
