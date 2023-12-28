#ifndef _UI_DISPLAY_SYSCALL_H_
#define _UI_DISPLAY_SYSCALL_H_ 1
#include <kernel/types.h>



typedef struct _UI_DISPLAY_USER_MODE{
	u32 width;
	u32 height;
	u32 freq;
} ui_display_user_mode_t;



typedef struct _UI_DISPLAY_USER_DATA{
	u32 index;
	ui_display_user_mode_t mode;
} ui_display_user_data_t;



typedef struct _UI_DISPLAY_USER_INFO{
	char manufacturer[4];
	u32 manufacturer_product_code;
	u32 serial_number;
	u8 video_interface;
	u32 screen_width_cm;
	u32 screen_height_cm;
	char name[14];
	u32 mode_count;
	ui_display_user_mode_t modes[];
} ui_display_user_info_t;



typedef struct _UI_DISPLAY_USER_FRAMEBUFFER{
	u64 size;
	u32 width;
	u32 height;
	u32 format;
} ui_display_user_framebuffer_t;



void ui_display_syscall_init(void);



#endif
