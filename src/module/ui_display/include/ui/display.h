#ifndef _UI_DISPLAY_H_
#define _UI_DISPLAY_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/types.h>
#include <ui/display_info.h>
#include <ui/framebuffer.h>



typedef struct _UI_DISPLAY{
	handle_t handle;
	const struct _UI_DISPLAY_DRIVER* driver;
	void* ctx;
	u32 index;
	ui_display_info_t* display_info;
	const ui_display_info_mode_t* mode;
	ui_framebuffer_t* framebuffer;
} ui_display_t;



typedef struct _UI_DISPLAY_DRIVER{
	const char* name;
	bool (*create_framebuffer)(ui_framebuffer_t*);
	void (*delete_framebuffer)(ui_framebuffer_t*);
	bool (*resize_framebuffer)(ui_display_t*);
	void (*flush_framebuffer)(ui_display_t*);
} ui_display_driver_t;



extern handle_type_t ui_display_handle_type;



ui_display_t* ui_display_create(const ui_display_driver_t* driver,void* ctx,u32 index,const u8* edid,u32 edid_length);



bool ui_display_set_mode(ui_display_t* display,const ui_display_info_mode_t* mode);



#endif
