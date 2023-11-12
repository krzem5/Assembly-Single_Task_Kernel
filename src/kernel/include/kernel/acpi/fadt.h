#ifndef _KERNEL_ACPI_FADT_H_
#define _KERNEL_ACPI_FADT_H_ 1
#include <kernel/acpi/structures.h>
#include <kernel/types.h>



void acpi_fadt_load(const acpi_fadt_t* fadt);



void KERNEL_NORETURN acpi_fadt_shutdown(_Bool restart);



void KERNEL_NORETURN _acpi_fadt_reboot(void);



#endif
