#ifndef _KERNEL_PCI_PCI_H_
#define _KERNEL_PCI_PCI_H_ 1
#include <kernel/handle/handle.h>
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



typedef struct _PCI_DEVICE_ADDRESS{
	u8 bus;
	u8 slot;
	u8 func;
} pci_device_address_t;



typedef struct _PCI_DEVICE{
	handle_t handle;
	pci_device_address_t address;
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
	u8 flags;
} pci_bar_t;



extern handle_type_t pci_device_handle_type;



static KERNEL_INLINE u8 pci_device_read_config8(u32 offset){
	io_port_out32(0xcf8,offset);
	return io_port_in8(0xcfc);
}



static KERNEL_INLINE u16 pci_device_read_config16(u32 offset){
	io_port_out32(0xcf8,offset);
	return io_port_in16(0xcfc);
}



static KERNEL_INLINE u32 pci_device_read_config32(u32 offset){
	io_port_out32(0xcf8,offset);
	return io_port_in32(0xcfc);
}



static KERNEL_INLINE void pci_device_write_config8(u32 offset,u8 value){
	io_port_out32(0xcf8,offset);
	io_port_out8(0xcfc,value);
}



static KERNEL_INLINE void pci_device_write_config16(u32 offset,u16 value){
	io_port_out32(0xcf8,offset);
	io_port_out16(0xcfc,value);
}



static KERNEL_INLINE void pci_device_write_config32(u32 offset,u32 value){
	io_port_out32(0xcf8,offset);
	io_port_out32(0xcfc,value);
}



static KERNEL_INLINE u32 pci_device_read_data_raw(const pci_device_address_t* device_address,u8 offset){
	return pci_device_read_config32((device_address->bus<<16)|(device_address->slot<<11)|(device_address->func<<8)|(offset&0xfc)|0x80000000);
}



static KERNEL_INLINE u32 pci_device_read_data(const pci_device_t* device,u8 offset){
	return pci_device_read_data_raw(&(device->address),offset);
}



static KERNEL_INLINE void pci_device_write_data_raw(const pci_device_address_t* device_address,u8 offset,u32 value){
	pci_device_write_config32((device_address->bus<<16)|(device_address->slot<<11)|(device_address->func<<8)|(offset&0xfc)|0x80000000,value);
}



static KERNEL_INLINE void pci_device_write_data(const pci_device_t* device,u8 offset,u32 value){
	pci_device_write_data_raw(&(device->address),offset,value);
}



static KERNEL_INLINE void pci_device_enable_io_access(const pci_device_t* device){
	pci_device_write_data(device,4,pci_device_read_data(device,4)|1);
}



static KERNEL_INLINE void pci_device_enable_memory_access(const pci_device_t* device){
	pci_device_write_data(device,4,pci_device_read_data(device,4)|2);
}



static KERNEL_INLINE void pci_device_enable_bus_mastering(const pci_device_t* device){
	pci_device_write_data(device,4,pci_device_read_data(device,4)|4);
}



_Bool pci_device_get_bar(const pci_device_t* device,u8 bar_index,pci_bar_t* out);



#endif
