#include <kernel/acpi/fadt.h>
#include <kernel/acpi/hmat.h>
#include <kernel/acpi/madt.h>
#include <kernel/acpi/slit.h>
#include <kernel/acpi/srat.h>
#include <kernel/acpi/structures.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "acpi"



void acpi_load(void){
	LOG("Loading ACPI RSDP...");
	const rsdp_t* rsdp=(void*)vmm_identity_map(kernel_data.rsdp_address,sizeof(rsdt_t));
	INFO("Found RSDP at %p (revision %u)",((u64)rsdp)-VMM_HIGHER_HALF_ADDRESS_OFFSET,rsdp->revision);
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
	rsdt=(void*)vmm_identity_map((u64)rsdt,((const rsdt_t*)vmm_identity_map((u64)rsdt,sizeof(rsdt_t)))->header.length);
	u32 entry_count=(rsdt->header.length-sizeof(rsdt_t))>>(2+is_xsdt);
	const madt_t* madt=NULL;
	const fadt_t* fadt=NULL;
	const hmat_t* hmat=NULL;
	const srat_t* srat=NULL;
	const slit_t* slit=NULL;
	for (u32 i=0;i<entry_count;i++){
		const sdt_t* sdt=(void*)(is_xsdt?rsdt->xsdt_data[i]:rsdt->rsdt_data[i]);
		sdt=(void*)vmm_identity_map((u64)sdt,((const sdt_t*)vmm_identity_map((u64)sdt,sizeof(sdt_t)))->length);
		if (sdt->signature==0x43495041){
			madt=(const madt_t*)sdt;
			INFO("Found MADT at %p",((u64)sdt)-VMM_HIGHER_HALF_ADDRESS_OFFSET);
		}
		else if (sdt->signature==0x50434146){
			fadt=(const fadt_t*)sdt;
			INFO("Found FADT at %p",((u64)sdt)-VMM_HIGHER_HALF_ADDRESS_OFFSET);
		}
		else if (sdt->signature==0x54414d48){
			hmat=(const hmat_t*)sdt;
			INFO("Found HMAT at %p",((u64)sdt)-VMM_HIGHER_HALF_ADDRESS_OFFSET);
		}
		else if (sdt->signature==0x54415253){
			srat=(const srat_t*)sdt;
			INFO("Found SRAT at %p",((u64)sdt)-VMM_HIGHER_HALF_ADDRESS_OFFSET);
		}
		else if (sdt->signature==0x54494c53){
			slit=(const slit_t*)sdt;
			INFO("Found SLIT at %p",((u64)sdt)-VMM_HIGHER_HALF_ADDRESS_OFFSET);
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
		if (slit){
			acpi_slit_load(slit);
		}
	}
}
