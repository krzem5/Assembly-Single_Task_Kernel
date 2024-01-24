#ifndef _KERNEL_PCI_MSIX_H_
#define _KERNEL_PCI_MSIX_H_ 1
#include <kernel/pci/pci.h>
#include <kernel/types.h>



#define MSIX_TABLE_ENTRY_CONTROL_FLAG_MASKED 1



typedef volatile struct KERNEL_PACKED _MSIX_TABLE_ENTRY{
	u64 msg_address;
	u32 msg_data;
	u32 control;
} msix_table_entry_t;



typedef struct _MSIX_TABLE{
	msix_table_entry_t* data;
	u16 length;
} msix_table_t;



_Bool pci_msix_load(const pci_device_t* device,msix_table_t* out);



_Bool pci_msix_redirect_entry(const msix_table_t* table,u16 vector,u8 irq);



#endif
