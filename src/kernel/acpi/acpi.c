#include <kernel/acpi/fadt.h>
#include <kernel/acpi/hmat.h>
#include <kernel/acpi/madt.h>
#include <kernel/acpi/slit.h>
#include <kernel/acpi/srat.h>
#include <kernel/acpi/structures.h>
#include <kernel/acpi/tpm2.h>
#include <kernel/kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/vmm.h>
#include <kernel/pci/pci.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "acpi"



KERNEL_EARLY_EARLY_INIT(){
	LOG("Loading ACPI RSDP...");
	const acpi_rsdp_t* rsdp=(void*)vmm_identity_map(kernel_data.rsdp_address,sizeof(acpi_rsdt_t));
	INFO("Found RSDP at %p (revision %u)",((u64)rsdp)-VMM_HIGHER_HALF_ADDRESS_OFFSET,rsdp->revision);
	bool is_xsdt=0;
	const acpi_rsdt_t* rsdt;
	if (!rsdp->revision||!rsdp->xsdt_address){
		INFO("Found RSDT at %p",rsdp->rsdt_address);
		rsdt=(void*)(u64)(rsdp->rsdt_address);
	}
	else{
		is_xsdt=1;
		INFO("Found XSDT at %p",rsdp->xsdt_address);
		rsdt=(void*)(rsdp->xsdt_address);
	}
	rsdt=(void*)vmm_identity_map((u64)rsdt,((const acpi_rsdt_t*)vmm_identity_map((u64)rsdt,sizeof(acpi_rsdt_t)))->header.length);
	u32 entry_count=(rsdt->header.length-sizeof(acpi_rsdt_t))>>(2+is_xsdt);
	const acpi_madt_t* madt=NULL;
	const acpi_fadt_t* fadt=NULL;
	const acpi_hmat_t* hmat=NULL;
	const acpi_srat_t* srat=NULL;
	const acpi_slit_t* slit=NULL;
	const acpi_tpm2_t* tpm2=NULL;
	const acpi_mcfg_t* mcfg=NULL;
	for (u32 i=0;i<entry_count;i++){
		const acpi_sdt_t* sdt=(void*)(is_xsdt?rsdt->xsdt_data[i]:rsdt->rsdt_data[i]);
		sdt=(void*)vmm_identity_map((u64)sdt,((const acpi_sdt_t*)vmm_identity_map((u64)sdt,sizeof(acpi_sdt_t)))->length);
		if (sdt->signature==0x43495041){
			madt=(const acpi_madt_t*)sdt;
			INFO("Found MADT at %p",((u64)sdt)-VMM_HIGHER_HALF_ADDRESS_OFFSET);
		}
		else if (sdt->signature==0x50434146){
			fadt=(const acpi_fadt_t*)sdt;
			INFO("Found FADT at %p",((u64)sdt)-VMM_HIGHER_HALF_ADDRESS_OFFSET);
		}
		else if (sdt->signature==0x54414d48){
			hmat=(const acpi_hmat_t*)sdt;
			INFO("Found HMAT at %p",((u64)sdt)-VMM_HIGHER_HALF_ADDRESS_OFFSET);
		}
		else if (sdt->signature==0x54415253){
			srat=(const acpi_srat_t*)sdt;
			INFO("Found SRAT at %p",((u64)sdt)-VMM_HIGHER_HALF_ADDRESS_OFFSET);
		}
		else if (sdt->signature==0x54494c53){
			slit=(const acpi_slit_t*)sdt;
			INFO("Found SLIT at %p",((u64)sdt)-VMM_HIGHER_HALF_ADDRESS_OFFSET);
		}
		else if (sdt->signature==0x324d5054){
			tpm2=(const acpi_tpm2_t*)sdt;
			INFO("Found TPM2 at %p",((u64)sdt)-VMM_HIGHER_HALF_ADDRESS_OFFSET);
		}
		else if (sdt->signature==0x4746434d){
			mcfg=(const acpi_mcfg_t*)sdt;
			INFO("Found MCFG at %p",((u64)sdt)-VMM_HIGHER_HALF_ADDRESS_OFFSET);
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
	if (tpm2){
		acpi_tpm2_load(tpm2);
	}
	if (mcfg){
		pci_set_pcie_table(mcfg);
	}
}
