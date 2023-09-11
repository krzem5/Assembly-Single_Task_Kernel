#include <kernel/acpi/fadt.h>
#include <kernel/acpi/hmat.h>
#include <kernel/acpi/madt.h>
#include <kernel/acpi/slit.h>
#include <kernel/acpi/srat.h>
#include <kernel/log/log.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
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
	u8 _padding[28];
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
		vmm_identity_map((void*)(range[0]),range[1]-range[0]);
		const u64* start=(void*)(range[0]);
		const u64* end=(void*)(range[1]);
		while (start!=end){
			if (start[0]==0x2052545020445352ull){
				rsdp=(void*)start;
				goto _rsdp_found;
			}
			start+=2;
		}
		range+=2;
	}
	panic("Unable to locate the RSDP",0);
_rsdp_found:
	INFO("Found RSDP at %p (revision %u)",rsdp,rsdp->revision);
	_Bool is_xsdt=0;
	const rsdt_t* rsdt;
	if (!rsdp->revision||!rsdp->xsdt_address){
		INFO("Found RSDT at %p",rsdp->rsdt_address);
		rsdt=(void*)(u64)(rsdp->rsdt_address);
	}
	else{
		is_xsdt=1;
		INFO("Found XSDT at %p",rsdp->xsdt_address);
		rsdt=(void*)(rsdp->xsdt_address);
	}
	vmm_identity_map(rsdt,sizeof(rsdt_t));
	u32 entry_count=(rsdt->header.length-sizeof(rsdt_t))>>(2+is_xsdt);
	vmm_identity_map(rsdt,rsdt->header.length);
	const sdt_t* madt=NULL;
	const sdt_t* fadt=NULL;
	const sdt_t* hmat=NULL;
	const sdt_t* srat=NULL;
	const sdt_t* slit=NULL;
	for (u32 i=0;i<entry_count;i++){
		const sdt_t* sdt=(void*)(is_xsdt?rsdt->xsdt_data[i]:rsdt->rsdt_data[i]);
		vmm_identity_map(sdt,sizeof(sdt_t));
		vmm_identity_map(sdt,sdt->length);
		if (sdt->signature==0x43495041){
			madt=sdt;
			INFO("Found MADT at %p",sdt);
		}
		else if (sdt->signature==0x50434146){
			fadt=sdt;
			INFO("Found FADT at %p",sdt);
		}
		else if (sdt->signature==0x54414d48){
			hmat=sdt;
			INFO("Found HMAT at %p",sdt);
		}
		else if (sdt->signature==0x54415253){
			srat=sdt;
			INFO("Found SRAT at %p",sdt);
		}
		else if (sdt->signature==0x54494c53){
			slit=sdt;
			INFO("Found SLIT at %p",sdt);
		}
	}
	if (madt){
		acpi_madt_load(madt);
	}
	if (fadt){
		acpi_fadt_load(fadt);
	}
	if (hmat){
		acpi_hmat_load(hmat);
	}
	if (srat){
		acpi_srat_load(srat);
	}
	if (slit){
		acpi_slit_load(slit);
	}
}
