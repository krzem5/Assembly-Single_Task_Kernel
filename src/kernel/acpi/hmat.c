#include <kernel/acpi/structures.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "hmat"



void KERNEL_EARLY_EXEC acpi_hmat_load(const acpi_hmat_t* hmat){
	LOG("Loading HMAT...");
}
