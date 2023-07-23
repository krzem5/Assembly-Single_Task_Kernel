#ifndef _KERNEL_ACPI_FADT_H_
#define _KERNEL_ACPI_FADT_H_ 1



void acpi_fadt_load(const void* fadt_ptr);



void acpi_fadt_shutdown(_Bool restart);



#endif
