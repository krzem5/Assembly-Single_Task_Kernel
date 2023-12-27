#ifndef _UI_DISPLAY_INFO_H_
#define _UI_DISPLAY_INFO_H_ 1
#include <kernel/types.h>



#define UI_DISPLAY_INFO_VIDEO_INTERFACE_UNKNOWN 0
#define UI_DISPLAY_INFO_VIDEO_INTERFACE_DVI 1
#define UI_DISPLAY_INFO_VIDEO_INTERFACE_HDMIA 2
#define UI_DISPLAY_INFO_VIDEO_INTERFACE_HDMIB 3
#define UI_DISPLAY_INFO_VIDEO_INTERFACE_MDDI 4
#define UI_DISPLAY_INFO_VIDEO_INTERFACE_DISPLAY_PORT 5
#define UI_DISPLAY_INFO_VIDEO_INTERFACE_ANALOG 255



typedef struct _UI_DISPLAY_INFO_MODE{
	struct _UI_DISPLAY_INFO_MODE* next;
	u32 width;
	u32 height;
	u32 freq;
} ui_display_info_mode_t;



typedef struct _UI_DISPLAY_INFO{
	u8* edid;
	u32 edid_length;
	char manufacturer[4];
	u32 manufacturer_product_code;
	u32 serial_number;
	u8 video_interface;
	u32 screen_width_cm;
	u32 screen_height_cm;
	char name[14];
	ui_display_info_mode_t* modes;
} ui_display_info_t;



void ui_display_info_init(void);



ui_display_info_t* ui_display_info_parse_edid(const u8* edid,u32 edid_length);



const ui_display_info_mode_t* ui_display_info_find_mode(const ui_display_info_t* display_info,u32 width,u32 height,u32 freq);



#endif
