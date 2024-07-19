#ifndef _KERNEL_PCI_PCI_H_
#define _KERNEL_PCI_PCI_H_ 1
#include <kernel/acpi/structures.h>
#include <kernel/handle/handle.h>
#include <kernel/io/io.h>
#include <kernel/types.h>



#define PCI_INTERRUPT_STATE_NONE 0
#define PCI_INTERRUPT_STATE_MSI 1
#define PCI_INTERRUPT_STATE_MSIX 2

#define PCI_BAR_FLAG_MEMORY 1

#define PCI_CAP_ID_MSI 5
#define PCI_CAP_ID_VENDOR 9
#define PCI_CAP_ID_MSIX 17



typedef struct _PCI_INTERRUPT_STATE{
	u8 state;
	union{
		struct{
			u8 offset;
		} msi;
		struct{
			u8 offset;
			u8 next_table_index;
			u16 table_size;
		} msix;
	};
} pci_interrupt_state_t;



typedef struct _PCI_DEVICE{
	handle_t handle;
	u64 _offset;
	u16 segment_group;
	u8 bus;
	u8 slot;
	u8 func;
	u16 device_id;
	u16 vendor_id;
	u8 class;
	u8 subclass;
	u8 progif;
	u8 revision_id;
	u8 header_type;
	u8 interrupt_line;
	pci_interrupt_state_t interrupt_state;
} pci_device_t;



typedef struct _PCI_BAR{
	u64 address;
	u64 size;
	u32 flags;
} pci_bar_t;



extern handle_type_t pci_device_handle_type;



void pci_set_pcie_table(const acpi_mcfg_t* mcfg);



bool pci_device_get_bar(const pci_device_t* device,u8 bar_index,pci_bar_t* out);



u32 pci_device_get_cap(const pci_device_t* device,u8 cap,u32 offset);



u8 pci_device_read_config8(u64 offset);



u16 pci_device_read_config16(u64 offset);



u32 pci_device_read_config32(u64 offset);



void pci_device_write_config8(u64 offset,u8 value);



void pci_device_write_config16(u64 offset,u16 value);



void pci_device_write_config32(u64 offset,u32 value);



static KERNEL_INLINE u32 KERNEL_NOCOVERAGE pci_device_read_data(const pci_device_t* device,u32 offset){
	return pci_device_read_config32(device->_offset|(offset&0xfffffffc));
}



static KERNEL_INLINE void KERNEL_NOCOVERAGE pci_device_write_data(const pci_device_t* device,u32 offset,u32 value){
	pci_device_write_config32(device->_offset|(offset&0xfffffffc),value);
}



void pci_device_enable_io_access(const pci_device_t* device);



void pci_device_enable_memory_access(const pci_device_t* device);



void pci_device_enable_bus_mastering(const pci_device_t* device);



#endif
