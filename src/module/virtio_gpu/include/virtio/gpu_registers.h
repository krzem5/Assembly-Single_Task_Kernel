#ifndef _VIRTIO_GPU_REGISTERS_H_
#define _VIRTIO_GPU_REGISTERS_H_ 1
#include <kernel/types.h>



#define VIRTIO_GPU_MAX_SCANOUTS 16

#define VIRTIO_GPU_NO_RESOURCE 0



// Features
#define VIRTIO_GPU_F_VIRGL 0
#define VIRTIO_GPU_F_EDID 1

// Registers
#define VIRTIO_GPU_REG_EVENTS_READ 0x00
#define VIRTIO_GPU_REG_EVENTS_CLEAR 0x04
#define VIRTIO_GPU_REG_NUM_SCANOUTS 0x08
#define VIRTIO_GPU_REG_NUM_CAPSETS 0x0c

// Events
#define VIRTIO_GPU_EVENT_DISPLAY 0x01

// Commands
#define VIRTIO_GPU_CMD_GET_DISPLAY_INFO 0x0100
#define VIRTIO_GPU_CMD_RESOURCE_CREATE_2D 0x0101
#define VIRTIO_GPU_CMD_RESOURCE_UNREF 0x0102
#define VIRTIO_GPU_CMD_SET_SCANOUT 0x0103
#define VIRTIO_GPU_CMD_RESOURCE_FLUSH 0x0104
#define VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D 0x0105
#define VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING 0x0106
#define VIRTIO_GPU_CMD_RESOURCE_DETACH_BACKING 0x0107
#define VIRTIO_GPU_CMD_GET_CAPSET_INFO 0x0108
#define VIRTIO_GPU_CMD_GET_CAPSET 0x0109
#define VIRTIO_GPU_CMD_GET_EDID 0x010a
#define VIRTIO_GPU_CMD_RESOURCE_ASSIGN_UUID 0x010b
#define VIRTIO_GPU_CMD_RESOURCE_CREATE_BLOB 0x010c
#define VIRTIO_GPU_CMD_SET_SCANOUT_BLOB 0x010d
#define VIRTIO_GPU_CMD_CTX_CREATE 0x0200
#define VIRTIO_GPU_CMD_CTX_DESTROY 0x0201
#define VIRTIO_GPU_CMD_CTX_ATTACH_RESOURCE 0x0202
#define VIRTIO_GPU_CMD_CTX_DETACH_RESOURCE 0x0203
#define VIRTIO_GPU_CMD_RESOURCE_CREATE_3D 0x0204
#define VIRTIO_GPU_CMD_TRANSFER_TO_HOST_3D 0x0205
#define VIRTIO_GPU_CMD_TRANSFER_FROM_HOST_3D 0x0206
#define VIRTIO_GPU_CMD_SUBMIT_3D 0x0207
#define VIRTIO_GPU_CMD_RESOURCE_MAP_BLOB 0x0208
#define VIRTIO_GPU_CMD_RESOURCE_UNMAP_BLOB 0x0209
#define VIRTIO_GPU_CMD_UPDATE_CURSOR 0x0300
#define VIRTIO_GPU_CMD_MOVE_CURSOR 0x0301
#define VIRTIO_GPU_RESP_OK_NODATA 0x1100
#define VIRTIO_GPU_RESP_OK_DISPLAY_INFO 0x1101
#define VIRTIO_GPU_RESP_OK_CAPSET_INFO 0x1102
#define VIRTIO_GPU_RESP_OK_CAPSET 0x1103
#define VIRTIO_GPU_RESP_OK_EDID 0x1104
#define VIRTIO_GPU_RESP_OK_RESOURCE_UUID 0x1105
#define VIRTIO_GPU_RESP_OK_MAP_INFO 0x1106
#define VIRTIO_GPU_RESP_ERR_UNSPEC 0x1200
#define VIRTIO_GPU_RESP_ERR_OUT_OF_MEMORY 0x1201
#define VIRTIO_GPU_RESP_ERR_INVALID_SCANOUT_ID 0x1202
#define VIRTIO_GPU_RESP_ERR_INVALID_RESOURCE_ID 0x1203
#define VIRTIO_GPU_RESP_ERR_INVALID_CONTEXT_ID 0x1204
#define VIRTIO_GPU_RESP_ERR_INVALID_PARAMETER 0x1205

// Flags
#define VIRTIO_GPU_FLAG_FENCE 0x01
#define VIRTIO_GPU_FLAG_INFO_RING_IDX 0x02

// GPU Resource Formats
#define VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM 1
#define VIRTIO_GPU_FORMAT_B8G8R8X8_UNORM 2
#define VIRTIO_GPU_FORMAT_A8R8G8B8_UNORM 3
#define VIRTIO_GPU_FORMAT_X8R8G8B8_UNORM 4
#define VIRTIO_GPU_FORMAT_R8G8B8A8_UNORM 67
#define VIRTIO_GPU_FORMAT_X8B8G8R8_UNORM 68
#define VIRTIO_GPU_FORMAT_A8B8G8R8_UNORM 121
#define VIRTIO_GPU_FORMAT_R8G8B8X8_UNORM 134

// GPU Resource flags
#define VIRTIO_GPU_RESOURCE_FLAG_Y_0_TOP 1

// Capset types
#define VIRTIO_GPU_CAPSET_VIRGL 1
#define VIRTIO_GPU_CAPSET_VIRGL2 2
#define VIRTIO_GPU_CAPSET_GFXSTREAM 3
#define VIRTIO_GPU_CAPSET_VENUS 4
#define VIRTIO_GPU_CAPSET_CROSS_DOMAIN 5



typedef u32 virtio_gpu_resource_id_t;



typedef struct KERNEL_PACKED _VIRTIO_GPU_CONTROL_HEADER{
	u32 type;
	u32 flags;
	u64 fence_id;
	u32 ctx_id;
	u8 ring_idx;
	u8 _padding[3];
} virtio_gpu_control_header_t;



typedef struct KERNEL_PACKED _VIRTIO_GPU_RECT{
	u32 x;
	u32 y;
	u32 width;
	u32 height;
} virtio_gpu_rect_t;



typedef struct KERNEL_PACKED _VIRTIO_GPU_RESP_DISPLAY_INFO{
	virtio_gpu_control_header_t header;
	struct KERNEL_PACKED{
		virtio_gpu_rect_t rect;
		u32 enabled;
		u32 flags;
	} displays[VIRTIO_GPU_MAX_SCANOUTS];
} virtio_gpu_resp_display_info_t;



typedef struct KERNEL_PACKED _VIRTIO_GPU_GET_EDID{
	virtio_gpu_control_header_t header;
	u32 scanout;
	u32 _padding;
} virtio_gpu_get_edid_t;



typedef struct KERNEL_PACKED _VIRTIO_GPU_RESP_EDID{
	virtio_gpu_control_header_t header;
	u32 size;
	u32 _padding;
	u8 edid[1024];
} virtio_gpu_resp_edid_t;



typedef struct KERNEL_PACKED _VIRTIO_GPU_RESOURCE_CREATE_2D{
	virtio_gpu_control_header_t header;
	virtio_gpu_resource_id_t resource_id;
	u32 format;
	u32 width;
	u32 height;
} virtio_gpu_resource_create_2d_t;



typedef struct KERNEL_PACKED _VIRTIO_GPU_MEM_ENTRY{
	u64 address;
	u32 length;
	u32 _padding;
} virtio_gpu_mem_entry_t;



typedef struct KERNEL_PACKED _VIRTIO_GPU_RESOURCE_ATTACH_BACKING{
	virtio_gpu_control_header_t header;
	virtio_gpu_resource_id_t resource_id;
	u32 entry_count;
	virtio_gpu_mem_entry_t entries[];
} virtio_gpu_resource_attach_backing_t;



typedef struct KERNEL_PACKED _VIRTIO_GPU_SET_SCANOUT{
	virtio_gpu_control_header_t header;
	virtio_gpu_rect_t rect;
	u32 scanout_id;
	virtio_gpu_resource_id_t resource_id;
} virtio_gpu_set_scanout_t;



typedef struct KERNEL_PACKED _VIRTIO_GPU_TRANSFER_TO_HOST_2D{
	virtio_gpu_control_header_t header;
	virtio_gpu_rect_t rect;
	u64 offset;
	virtio_gpu_resource_id_t resource_id;
	u32 _padding;
} virtio_gpu_transfer_to_host_2d_t;



typedef struct KERNEL_PACKED _VIRTIO_GPU_RESOURCE_FLUSH{
	virtio_gpu_control_header_t header;
	virtio_gpu_rect_t rect;
	virtio_gpu_resource_id_t resource_id;
	u32 _padding;
} virtio_gpu_resource_flush_t;



typedef struct KERNEL_PACKED _VIRTIO_GPU_GET_CAPSET_INFO{
	virtio_gpu_control_header_t header;
	u32 capset_index;
	u32 _padding;
} virtio_gpu_get_capset_info_t;



typedef struct KERNEL_PACKED _VIRTIO_GPU_RESP_CAPSET_INFO{
	virtio_gpu_control_header_t header;
	u32 capset_id;
	u32 capset_max_version;
	u32 capset_max_size;
	u32 _padding;
} virtio_gpu_resp_capset_info_t;



typedef struct KERNEL_PACKED _VIRTIO_GPU_GET_CAPSET{
	virtio_gpu_control_header_t header;
	u32 capset_id;
	u32 capset_version;
} virtio_gpu_get_capset_t;



typedef struct KERNEL_PACKED _VIRTIO_GPU_RESP_CAPSET{
	virtio_gpu_control_header_t header;
	u8 capset_data[];
} virtio_gpu_resp_capset_t;



typedef struct KERNEL_PACKED _VIRTIO_GPU_CTX_CREATE{
	virtio_gpu_control_header_t header;
	u32 debug_name_length;
	u32 context_init;
	char debug_name[64];
} virtio_gpu_ctx_create_t;



typedef struct KERNEL_PACKED _VIRTIO_GPU_CMD_SUBMIT{
	virtio_gpu_control_header_t header;
	u32 size;
	u32 _padding;
} virtio_gpu_cmd_submit_t;



typedef struct KERNEL_PACKED _VIRTIO_GPU_RESOURCE_CREATE_3D{
	virtio_gpu_control_header_t header;
	u32 resource_id;
	u32 target;
	u32 format;
	u32 bind;
	u32 width;
	u32 height;
	u32 depth;
	u32 array_size;
	u32 last_level;
	u32 nr_samples;
	u32 flags;
	u32 padding;
} virtio_gpu_resource_create_3d_t;



typedef struct KERNEL_PACKED _VIRTIO_GPU_CTX_ATACH_RESOURCE{
	virtio_gpu_control_header_t header;
	u32 resource_id;
	u32 _padding;
} virtio_gpu_ctx_attach_resource_t;



#endif
