#ifndef _VIRTIO_VIRTIO_H_
#define _VIRTIO_VIRTIO_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/lock/rwlock.h>
#include <kernel/mp/event.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <virtio/registers.h>



typedef u16 virtio_device_type_t;



typedef u64 virtio_field_t;



typedef struct _VIRTIO_BUFFER{
	u64 address;
	u32 length;
} virtio_buffer_t;



typedef struct _VIRTIO_DEVICE{
	handle_t handle;
	virtio_device_type_t type;
	u16 index;
	bool is_legacy;
	bool events_negotiated;
	u8 irq;
	u16 queue_msix_vector;
	rwlock_t lock;
	virtio_field_t common_field;
	virtio_field_t notify_field;
	virtio_field_t isr_field;
	virtio_field_t device_field;
	u32 notify_off_multiplier;
	struct _VIRTIO_QUEUE* queues;
} virtio_device_t;



typedef struct _VIRTIO_QUEUE{
	struct _VIRTIO_QUEUE* next;
	const virtio_device_t* device;
	u16 index;
	u16 size;
	u16 notify_offset;
	u16 first_free_index;
	u16 last_used_index;
	virtio_queue_descriptor_t* descriptors;
	virtio_queue_available_t* available;
	virtio_queue_event_t* available_used_event;
	virtio_queue_used_t* used;
	virtio_queue_event_t* used_available_event;
	event_t* event;
} virtio_queue_t;



typedef struct _VIRTIO_DEVICE_DRIVER{
	const char* name;
	virtio_device_type_t type;
	bool is_legacy;
	u64 features;
	bool (*init)(virtio_device_t*,u64);
} virtio_device_driver_t;



typedef struct _VIRTIO_DEVICE_DRIVER_NODE{
	rb_tree_node_t rb_node;
	const virtio_device_driver_t* driver;
} virtio_device_driver_node_t;



bool virtio_register_device_driver(const virtio_device_driver_t* driver);



bool virtio_unregister_device_driver(const virtio_device_driver_t* driver);



u64 virtio_read(virtio_field_t field,u8 size);



void virtio_write(virtio_field_t field,u8 size,u32 value);



virtio_queue_t* virtio_init_queue(virtio_device_t* device,u16 index);



void virtio_queue_transfer(virtio_queue_t* queue,const virtio_buffer_t* buffers,u16 tx_count,u16 rx_count);



void virtio_queue_wait(virtio_queue_t* queue);



u32 virtio_queue_pop(virtio_queue_t* queue,u32* length);



#endif
