#include <kernel/acpi/structures.h>
#include <kernel/log/log.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "tpm2"



void KERNEL_EARLY_EXEC acpi_tpm2_load(const acpi_tpm2_t* tpm2){
	LOG("Loading TPM2...");
	ERROR("start_method=%u",tpm2->start_method);
}
