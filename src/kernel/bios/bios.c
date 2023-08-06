#include <kernel/log/log.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
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
	};
} smbios_header_t;



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



void bios_get_system_data(void){
	LOG("Loading BIOS data...");
	INFO("Searching memory range %p - %p...",SMBIOS_MEMORY_REGION_START,SMBIOS_MEMORY_REGION_END);
	const smbios_t* smbios=NULL;
	const u32* start=VMM_TRANSLATE_ADDRESS(SMBIOS_MEMORY_REGION_START);
	const u32* end=VMM_TRANSLATE_ADDRESS(SMBIOS_MEMORY_REGION_END);
	while (start!=end){
		if (start[0]==0x5f4d535f){
			smbios=(const smbios_t*)start;
			goto _smbios_found;
		}
		start+=4;
	}
	ERROR("SMBIOS not found");
	for (;;);
_smbios_found:
	INFO("Found SMBIOS at %p (revision %u.%u)",VMM_TRANSLATE_ADDRESS_REVERSE(smbios),smbios->major_version,smbios->minor_version);
	INFO("SMBIOS table: %p - %p",smbios->table_address,smbios->table_address+smbios->table_length);
	for (u64 offset=smbios->table_address;offset<smbios->table_address+smbios->table_length;){
		const smbios_header_t* header=VMM_TRANSLATE_ADDRESS(offset);
		switch (header->type){
			case 0:
				INFO("%s, %s",_get_header_string(header,header->bios_information.vendor),_get_header_string(header,header->bios_information.bios_version));
				break;
			case 1:
				INFO("%s, %s, %s",_get_header_string(header,header->system_information.manufacturer),_get_header_string(header,header->system_information.product_name),_get_header_string(header,header->system_information.version));
				INFO("%s, %x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x, %u",_get_header_string(header,header->system_information.manufacturer),
					header->system_information.uuid[0],
					header->system_information.uuid[1],
					header->system_information.uuid[2],
					header->system_information.uuid[3],
					header->system_information.uuid[4],
					header->system_information.uuid[5],
					header->system_information.uuid[6],
					header->system_information.uuid[7],
					header->system_information.uuid[8],
					header->system_information.uuid[9],
					header->system_information.uuid[10],
					header->system_information.uuid[11],
					header->system_information.uuid[12],
					header->system_information.uuid[13],
					header->system_information.uuid[14],
					header->system_information.uuid[15],
					header->system_information.wakeup_type
				);
				// System information
				// wakeup_type=0 <=> Reserved
				// wakeup_type=1 <=> Other
				// wakeup_type=2 <=> Unknown
				// wakeup_type=3 <=> APM Timer
				// wakeup_type=4 <=> Modem Ring
				// wakeup_type=5 <=> LAN Remote
				// wakeup_type=6 <=> Power Switch
				// wakeup_type=7 <=> PCI PME#
				// wakeup_type=8 <=> AC Power Restored
				break;
		}
		for (u8 i=1;_get_header_string(header,i)[0];i++){
			WARN("[%u:%u] %s",header->type,i,_get_header_string(header,i));
		}
		offset+=_get_header_length(header);
	}
	for (;;);
}
