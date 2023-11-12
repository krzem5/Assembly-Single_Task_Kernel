#ifndef _KERNEL_ACPI_STRUCTURES_H_
#define _KERNEL_ACPI_STRUCTURES_H_ 1
#include <kernel/types.h>



#define MADT_ENTRY_TYPE_LAPIC 0
#define MADT_ENTRY_TYPE_IOAPIC 1
#define MADT_ENTRY_TYPE_ISO 2
#define MADT_ENTRY_TYPE_LAPIC_OVERRIDE 5

#define SRAT_ENTRY_TYPE_PROCESSOR 0
#define SRAT_ENTRY_TYPE_MEMORY 1

#define SRAT_ENTRY_PROCESSOR_FLAG_PRESENT 1
#define SRAT_ENTRY_MEMORY_FLAG_PRESENT 1



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



typedef struct __attribute__((packed)) _FADT{
	sdt_t header;
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
} fadt_t;



typedef struct __attribute__((packed)) _DSDT{
	sdt_t header;
	u8 data[];
} dsdt_t;



typedef struct __attribute__((packed)) _HMAT{
	sdt_t header;
} hmat_t;



typedef struct __attribute__((packed)) _MADT{
	sdt_t header;
	u32 lapic;
	u8 _padding[4];
	u8 entries[];
} madt_t;



typedef struct __attribute__((packed)) _MADT_ENTRY{
	u8 type;
	u8 length;
	union{
		struct __attribute__((packed)){
			u8 processor_id;
			u8 lapic_id;
			u32 flags;
		} lapic;
		struct __attribute__((packed)){
			u8 apic_id;
			u8 _padding;
			u32 address;
			u32 gsi_base;
		} io_apic;
		struct __attribute__((packed)){
			u8 _padding;
			u8 irq;
			u32 gsi;
			u16 flags;
		} iso;
		struct __attribute__((packed)){
			u8 _padding[2];
			u64 lapic;
		} lapic_override;
	};
} madt_entry_t;



typedef struct __attribute__((packed)) _SLIT{
	sdt_t header;
	u64 locality_count;
	u8 data[];
} slit_t;



typedef struct __attribute__((packed)) _SRAT{
	sdt_t header;
	u8 _padding[12];
	u8 data[];
} srat_t;



typedef struct __attribute__((packed)) _SRAT_ENTRY{
	u8 type;
	u8 length;
	union{
		struct __attribute__((packed)){
			u8 proximity_domain_low;
			u8 apic_id;
			u32 flags;
			u8 sapic_eid;
			u8 proximity_domain_high[3];
			u32 clock_domain;
		} processor;
		struct __attribute__((packed)){
			u32 proximity_domain;
			u8 _padding[2];
			u64 base_address;
			u64 length;
			u8 _padding2[4];
			u32 flags;
		} memory;
	};
} srat_entry_t;



#endif
