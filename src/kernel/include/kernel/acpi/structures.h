#ifndef _KERNEL_ACPI_STRUCTURES_H_
#define _KERNEL_ACPI_STRUCTURES_H_ 1
#include <kernel/types.h>



#define ACPI_MADT_ENTRY_TYPE_LAPIC 0
#define ACPI_MADT_ENTRY_TYPE_IOAPIC 1
#define ACPI_MADT_ENTRY_TYPE_ISO 2
#define ACPI_MADT_ENTRY_TYPE_LAPIC_OVERRIDE 5

#define ACPI_SRAT_ENTRY_TYPE_PROCESSOR 0
#define ACPI_SRAT_ENTRY_TYPE_MEMORY 1

#define ACPI_SRAT_ENTRY_PROCESSOR_FLAG_PRESENT 1
#define ACPI_SRAT_ENTRY_MEMORY_FLAG_PRESENT 1

#define ACPI_TPM2_START_METHOD_MEMORY_MAPPED 6



typedef struct KERNEL_PACKED _ACPI_RSDP{
	u64 signature;
	u8 _padding[7];
	u8 revision;
	u32 rsdt_address;
	u8 _padding2[4];
	u64 xsdt_address;
} acpi_rsdp_t;



typedef struct KERNEL_PACKED _ACPI_SDT{
	u32 signature;
	u32 length;
	u8 _padding[28];
} acpi_sdt_t;



typedef struct KERNEL_PACKED _ACPI_RSDT{
	acpi_sdt_t header;
	union{
		u32 rsdt_data[0];
		u64 xsdt_data[0];
	};
} acpi_rsdt_t;



typedef struct KERNEL_PACKED _ACPI_FADT{
	acpi_sdt_t header;
	u8 _padding[4];
	u32 dsdt;
	u8 _padding2[2];
	u16 sci_int;
	u32 smi_command_port;
	u8 acpi_enable;
	u8 acpi_disable;
	u8 _padding3[10];
	u32 pm1a_control_block;
	u32 pm1b_control_block;
} acpi_fadt_t;



typedef struct KERNEL_PACKED _ACPI_DSDT{
	acpi_sdt_t header;
	u8 data[];
} acpi_dsdt_t;



typedef struct KERNEL_PACKED _ACPI_HMAT{
	acpi_sdt_t header;
} acpi_hmat_t;



typedef struct KERNEL_PACKED _ACPI_MADT{
	acpi_sdt_t header;
	u32 lapic;
	u8 _padding[4];
	u8 entries[];
} acpi_madt_t;



typedef struct KERNEL_PACKED _ACPI_MADT_ENTRY{
	u8 type;
	u8 length;
	union{
		struct KERNEL_PACKED{
			u8 processor_id;
			u8 lapic_id;
			u32 flags;
		} lapic;
		struct KERNEL_PACKED{
			u8 apic_id;
			u8 _padding;
			u32 address;
			u32 gsi_base;
		} io_apic;
		struct KERNEL_PACKED{
			u8 _padding;
			u8 irq;
			u32 gsi;
			u16 flags;
		} iso;
		struct KERNEL_PACKED{
			u8 _padding[2];
			u64 lapic;
		} lapic_override;
	};
} acpi_madt_entry_t;



typedef struct KERNEL_PACKED _ACPI_SLIT{
	acpi_sdt_t header;
	u64 locality_count;
	u8 data[];
} acpi_slit_t;



typedef struct KERNEL_PACKED _ACPI_SRAT{
	acpi_sdt_t header;
	u8 _padding[12];
	u8 data[];
} acpi_srat_t;



typedef struct KERNEL_PACKED _ACPI_SRAT_ENTRY{
	u8 type;
	u8 length;
	union{
		struct KERNEL_PACKED{
			u8 proximity_domain_low;
			u8 apic_id;
			u32 flags;
			u8 sapic_eid;
			u8 proximity_domain_high[3];
			u32 clock_domain;
		} processor;
		struct KERNEL_PACKED{
			u32 proximity_domain;
			u8 _padding[2];
			u64 base_address;
			u64 length;
			u8 _padding2[4];
			u32 flags;
		} memory;
	};
} acpi_srat_entry_t;



typedef struct KERNEL_PACKED _ACPI_TPM2{
	acpi_sdt_t header;
	u16 platform_class;
	u8 _padding[2];
	u64 control_area_address;
	u32 start_method;
	u8 start_method_parameters[12];
} acpi_tpm2_t;



#endif
