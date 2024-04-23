#ifndef _KERNEL_AML_BUS_H_
#define _KERNEL_AML_BUS_H_ 1
#include <kernel/aml/namespace.h>
#include <kernel/handle/handle.h>
#include <kernel/types.h>



#define AML_BUS_RESOURCE_TYPE_NONE 0
#define AML_BUS_RESOURCE_TYPE_MEMORY_REGION 1
#define AML_BUS_RESOURCE_TYPE_INTERRUPT 2
#define AML_BUS_RESOURCE_TYPE_IO_PORT 3
#define AML_BUS_RESOURCE_TYPE_ADDRESS_SPACE 4

#define AML_BUS_RESOURCE_INTERRUPT_FLAG_EDGE_TRIGGER 1
#define AML_BUS_RESOURCE_INTERRUPT_FLAG_ACTIVE_LOW 2
#define AML_BUS_RESOURCE_INTERRUPT_FLAG_SHARED 4
#define AML_BUS_RESOURCE_INTERRUPT_FLAG_WAKE_CAPALE 8

#define AML_BUS_RESOURCE_ADDRESS_SPACE_TYPE_MEMORY 0
#define AML_BUS_RESOURCE_ADDRESS_SPACE_TYPE_IO 1

#define AML_BUS_RESOURCE_ADDRESS_SPACE_FLAG_MIN_FIXED 1
#define AML_BUS_RESOURCE_ADDRESS_SPACE_FLAG_MAX_FIXED 2

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
			bool writable;
		} memory_region;
		struct{
			u32 pin;
			u32 flags;
		} interrupt;
		struct{
			u16 base_min;
			u16 base_max;
			u8 base_alignment;
			u8 length;
			bool decode_all_address_bits;
		} io_port;
		struct{
			u8 type;
			u8 flags;
			u8 extra_flags;
			u64 granularity;
			u64 min;
			u64 max;
			u64 translation_offset;
			u64 length;
		} address_space;
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
	const struct _AML_BUS_DEVICE_DRIVER* driver;
	void* extra_data;
} aml_bus_device_t;



typedef struct _AML_BUS_DEVICE_DRIVER{
	const char* name;
	u32 address_type;
	union{
		u64 adr;
		u64 hid;
		const char* hid_str;
	} address;
	bool (*init)(aml_bus_device_t*);
	void (*deinit)(aml_bus_device_t*);
} aml_bus_device_driver_t;



typedef struct _AML_BUS_DEVICE_DRIVER_NODE{
	struct _AML_BUS_DEVICE_DRIVER_NODE* prev;
	struct _AML_BUS_DEVICE_DRIVER_NODE* next;
	const aml_bus_device_driver_t* driver;
} aml_bus_device_driver_node_t;



extern handle_type_t aml_bus_device_handle_type;



void aml_bus_scan(void);



const aml_bus_device_resource_t* aml_bus_device_get_resource(aml_bus_device_t* device,u32 type,u32 index);



void aml_bus_register_device_driver(const aml_bus_device_driver_t* driver);



void aml_bus_unregister_device_driver(const aml_bus_device_driver_t* driver);



#endif
