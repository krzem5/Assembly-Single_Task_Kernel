#ifndef _UI_DISPLAY_H_
#define _UI_DISPLAY_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/types.h>



typedef struct _UI_DISPLAY_MODE{
	struct _UI_DISPLAY_MODE* next;
	u32 width;
	u32 height;
	u32 freq;
} ui_display_mode_t;



typedef struct _UI_DISPLAY{
	handle_t handle;
	const struct _UI_DISPLAY_DRIVER* driver;
	void* ctx;
	u32 index;
	u32 edid_length;
	u8* edid;
	ui_display_mode_t* modes;
	const ui_display_mode_t* selected_mode;
	void* framebuffer;
} ui_display_t;



typedef struct _UI_DISPLAY_DRIVER{
	const char* name;
	_Bool (*resize)(ui_display_t*);
} ui_display_driver_t;



void ui_display_init(void);



ui_display_t* ui_add_display(const ui_display_driver_t* driver,void* ctx,u32 index,const u8* edid,u32 edid_length);



#endif
