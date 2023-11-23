#ifndef _KERNEL_ACPI_FADT_H_
#define _KERNEL_ACPI_FADT_H_ 1
#include <kernel/acpi/structures.h>
#include <kernel/types.h>



extern const acpi_fadt_t* acpi_fadt;
extern const acpi_dsdt_t* acpi_dsdt;



void acpi_fadt_load(const acpi_fadt_t* fadt);



#endif
