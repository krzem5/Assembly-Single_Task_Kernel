#include <kernel/bios/bios.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "bios"



#define SMBIOS_MEMORY_REGION_START 0x0f0000
#define SMBIOS_MEMORY_REGION_END 0x100000



typedef struct __attribute__((packed)) _SMBIOS{
	u32 entry_point_string;
	u8 _padding[2];
	u8 major_version;
	u8 minor_version;
	u8 _padding2[14];
	u16 table_length;
	u32 table_address;
} smbios_t;



typedef struct __attribute__((packed)) _SMBIO_HEADER{
	u8 type;
	u8 length;
	u8 _padding[2];
	union{
		struct __attribute__((packed)){
			u8 vendor;
			u8 bios_version;
		} bios_information;
		struct __attribute__((packed)){
			u8 manufacturer;
			u8 product_name;
			u8 version;
			u8 serial_number;
			u8 uuid[16];
			u8 wakeup_type;
		} system_information;
		struct __attribute__((packed)){
			u8 _padding[3];
			u8 serial_number;
		} baseboard_information;
	};
} smbios_header_t;



bios_data_t bios_data;



static u64 _get_header_length(const smbios_header_t* header){
	const u8* ptr=((const u8*)header)+header->length;
	while (ptr[0]||ptr[1]){
		ptr++;
	}
	return (u64)(ptr-((const u8*)header)+2);
}



static const char* _get_header_string(const smbios_header_t* header,u8 index){
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



static char* _duplicate_string(const char* str){
	u32 length=0;
	do{
		length++;
	} while (str[length-1]);
	char* out=kmm_alloc(length);
	memcpy(out,str,length);
	return out;
}



void bios_get_system_data(void){
	LOG("Loading BIOS data...");
	INFO("Searching memory range %p - %p...",SMBIOS_MEMORY_REGION_START,SMBIOS_MEMORY_REGION_END);
	vmm_identity_map((void*)SMBIOS_MEMORY_REGION_START,SMBIOS_MEMORY_REGION_END-SMBIOS_MEMORY_REGION_START);
	const smbios_t* smbios=NULL;
	for (const u32* start=(void*)SMBIOS_MEMORY_REGION_START;start<(const u32*)SMBIOS_MEMORY_REGION_END;start+=4){
		if (start[0]==0x5f4d535f){
			smbios=(const smbios_t*)start;
			goto _smbios_found;
		}
		start+=4;
	}
	ERROR("SMBIOS not found");
	for (;;);
_smbios_found:
	INFO("Found SMBIOS at %p (revision %u.%u)",smbios,smbios->major_version,smbios->minor_version);
	vmm_identity_map((void*)(u64)(smbios->table_address),smbios->table_length);
	INFO("SMBIOS table: %p - %p",smbios->table_address,smbios->table_address+smbios->table_length);
	memset(&bios_data,0,sizeof(bios_data_t));
	_Bool serial_number_found=0;
	for (u64 offset=smbios->table_address;offset<smbios->table_address+smbios->table_length;){
		const smbios_header_t* header=(void*)offset;
		switch (header->type){
			case 0:
				bios_data.bios_vendor=_duplicate_string(_get_header_string(header,header->bios_information.vendor));
				bios_data.bios_version=_duplicate_string(_get_header_string(header,header->bios_information.bios_version));
				break;
			case 1:
				bios_data.manufacturer=_duplicate_string(_get_header_string(header,header->system_information.manufacturer));
				bios_data.product=_duplicate_string(_get_header_string(header,header->system_information.product_name));
				bios_data.version=_duplicate_string(_get_header_string(header,header->system_information.version));
				if (!serial_number_found){
					bios_data.serial_number=_duplicate_string(_get_header_string(header,header->system_information.serial_number));
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
					default:
						bios_data.wakeup_type=BIOS_DATA_WAKEUP_TYPE_UNKNOWN;
						break;
				}
				break;
			case 2:
				bios_data.serial_number=_duplicate_string(_get_header_string(header,header->baseboard_information.serial_number));
				serial_number_found=1;
				break;
		}
		offset+=_get_header_length(header);
	}
	INFO("BIOS data:");
	INFO("  BIOS vendor: %s",bios_data.bios_vendor);
	INFO("  BIOS version: %s",bios_data.bios_version);
	INFO("  Manufacturer: %s",bios_data.manufacturer);
	INFO("  Product: %s",bios_data.product);
	INFO("  Version: %s",bios_data.version);
	INFO("  Serial number: %s",bios_data.serial_number);
	INFO("  UUID: %x%x%x%x-%x%x-%x%x-%x%x-%x%x%x%x%x%x",
		bios_data.uuid[0],
		bios_data.uuid[1],
		bios_data.uuid[2],
		bios_data.uuid[3],
		bios_data.uuid[4],
		bios_data.uuid[5],
		bios_data.uuid[6],
		bios_data.uuid[7],
		bios_data.uuid[8],
		bios_data.uuid[9],
		bios_data.uuid[10],
		bios_data.uuid[11],
		bios_data.uuid[12],
		bios_data.uuid[13],
		bios_data.uuid[14],
		bios_data.uuid[15]
	);
	switch (bios_data.wakeup_type){
		case BIOS_DATA_WAKEUP_TYPE_UNKNOWN:
			INFO("  Last wakeup: Unknown");
			break;
		case BIOS_DATA_WAKEUP_TYPE_POWER_SWITCH:
			INFO("  Last wakeup: Power switch");
			break;
		case BIOS_DATA_WAKEUP_TYPE_AC_POWER:
			INFO("  Last wakeup: AC power");
			break;
	}
}
