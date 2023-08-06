#include <kernel/acpi/fadt.h>
#include <kernel/acpi/madt.h>
#include <kernel/log/log.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "acpi"



typedef struct __attribute__((packed)) _RSDP{
	u64 signature;
	u8 _padding[7];
	u8 revision;
	u32 rsdt_address;
	u8 _padding2[4];
	u64 xsdt_address;
} rsdp_t;



typedef struct __attribute__((packed)) _SDT{
	u32 signature;
	u32 length;
	u8 revision;
	u8 _padding[27];
} sdt_t;



typedef struct __attribute__((packed)) _RSDT{
	sdt_t header;
	union{
		u32 rsdt_data[0];
		u64 xsdt_data[0];
	};
} rsdt_t;



static const u64 _rsdp_search_regions[]={
	0x00080000,0x000a0000,
	0x000e0000,0x00100000,
	0x00000000,0x00000000
};



void acpi_load(void){
	LOG("Loading ACPI RSDP...");
	const u64* range=_rsdp_search_regions;
	const rsdp_t* rsdp=NULL;
	while (range[0]){
		INFO("Searching memory range %p - %p...",range[0],range[1]);
		const u64* start=VMM_TRANSLATE_ADDRESS(range[0]);
		const u64* end=VMM_TRANSLATE_ADDRESS(range[1]);
		while (start!=end){
			if (start[0]==0x2052545020445352ull){
				rsdp=(void*)start;
				goto _rsdp_found;
			}
			start+=2;
		}
		range+=2;
	}
	ERROR("Unable to locate the RSDP");
	for (;;);
_rsdp_found:
	INFO("Found RSDP at %p (revision %u)",VMM_TRANSLATE_ADDRESS_REVERSE(rsdp),rsdp->revision);
	_Bool is_xsdt=0;
	const rsdt_t* rsdt;
	if (!rsdp->revision||!rsdp->xsdt_address){
		INFO("Found RSDT at %p",VMM_TRANSLATE_ADDRESS_REVERSE(rsdp->rsdt_address));
		rsdt=VMM_TRANSLATE_ADDRESS(rsdp->rsdt_address);
	}
	else{
		is_xsdt=1;
		INFO("Found XSDT at %p",VMM_TRANSLATE_ADDRESS_REVERSE(rsdp->xsdt_address));
		rsdt=VMM_TRANSLATE_ADDRESS(rsdp->xsdt_address);
	}
	u32 entry_count=(rsdt->header.length-sizeof(rsdt_t))>>(2+is_xsdt);
	for (u32 i=0;i<entry_count;i++){
		const sdt_t* sdt=VMM_TRANSLATE_ADDRESS((is_xsdt?rsdt->xsdt_data[i]:rsdt->rsdt_data[i]));
		if (sdt->signature==0x43495041){
			INFO("Found MADT at %p",VMM_TRANSLATE_ADDRESS_REVERSE(sdt));
			acpi_madt_load(sdt);
		}
		else if (sdt->signature==0x50434146){
			INFO("Found FADT at %p",VMM_TRANSLATE_ADDRESS_REVERSE(sdt));
			acpi_fadt_load(sdt);
		}
	}
}
