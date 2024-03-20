#ifndef _KERNEL_AML_BUS_H_
#define _KERNEL_AML_BUS_H_ 1
#include <kernel/aml/namespace.h>
#include <kernel/handle/handle.h>
#include <kernel/types.h>



#define AML_BUS_RESOURCE_TYPE_NONE 0
#define AML_BUS_RESOURCE_TYPE_MEMORY_REGION 1

#define AML_BUS_ADDRESS_TYPE_ADR 0
#define AML_BUS_ADDRESS_TYPE_HID 1
#define AML_BUS_ADDRESS_TYPE_HID_STR 2

#define AML_BUS_UID_TYPE_INT 0
#define AML_BUS_UID_TYPE_STR 1

#define AML_BUS_DEVICE_SLOT_UNKNOWN 0xffffffffffffffffull



typedef struct _AML_BUS_DEVICE_RESOURCE{
	struct _AML_BUS_DEVICE_RESOURCE* prev;
	struct _AML_BUS_DEVICE_RESOURCE* next;
	u32 type;
	union{
		struct{
			u32 base;
			u32 size;
			_Bool writable;
		} memory_region;
	};
} aml_bus_device_resource_t;



typedef struct _AML_BUS_DEVICE{
	u32 address_type;
	u32 uid_type;
	union{
		u64 adr;
		u64 hid;
		string_t* hid_str;
	} address;
	handle_t handle;
	aml_namespace_t* device;
	struct _AML_BUS_DEVICE* parent;
	u64 slot_unique_id;
	union{
		u64 uid;
		string_t* uid_str;
	};
	aml_bus_device_resource_t* resource_head;
	aml_bus_device_resource_t* resource_tail;
} aml_bus_device_t;



extern handle_type_t aml_bus_device_handle_type;



void aml_bus_scan(void);



#endif
