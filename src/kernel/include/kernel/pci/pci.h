#ifndef _KERNEL_PCI_PCI_H_
#define _KERNEL_PCI_PCI_H_ 1
#include <kernel/io/io.h>
#include <kernel/types.h>



#define PCI_INTERRUPT_STATE_NONE 0
#define PCI_INTERRUPT_STATE_MSI 1
#define PCI_INTERRUPT_STATE_MSIX 2

#define PCI_BAR_FLAG_MEMORY 1



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
	void* address;
	u64 size;
	u8 flags;
} pci_bar_t;



static inline u32 pci_device_read_data(const pci_device_t* device,u8 offset){
	io_port_out32(0xcf8,(device->bus<<16)|(device->slot<<11)|(device->func<<8)|(offset&0xfc)|0x80000000);
	return io_port_in32(0xcfc);
}



static inline void pci_device_write_data(const pci_device_t* device,u8 offset,u32 value){
	io_port_out32(0xcf8,(device->bus<<16)|(device->slot<<11)|(device->func<<8)|(offset&0xfc)|0x80000000);
	io_port_out32(0xcfc,value);
}



static inline void pci_device_enable_io_access(const pci_device_t* device){
	pci_device_write_data(device,4,pci_device_read_data(device,4)|1);
}



static inline void pci_device_enable_memory_access(const pci_device_t* device){
	pci_device_write_data(device,4,pci_device_read_data(device,4)|2);
}



static inline void pci_device_enable_bus_mastering(const pci_device_t* device){
	pci_device_write_data(device,4,pci_device_read_data(device,4)|4);
}



void pci_enumerate(_Bool early_boot);



_Bool pci_device_get_bar(const pci_device_t* device,u8 bar_index,pci_bar_t* out);



#endif
