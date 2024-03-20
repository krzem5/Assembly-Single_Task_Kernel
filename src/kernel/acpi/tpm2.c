#include <kernel/acpi/structures.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "tpm2"



const acpi_tpm2_t* acpi_tpm2=NULL;



void KERNEL_EARLY_EXEC acpi_tpm2_load(const acpi_tpm2_t* tpm2){
	LOG("Loading TPM2...");
	acpi_tpm2=tpm2;
}
