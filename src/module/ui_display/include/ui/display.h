#ifndef _UI_DISPLAY_H_
#define _UI_DISPLAY_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/types.h>
#include <ui/display_info.h>



typedef struct _UI_DISPLAY{
	handle_t handle;
	const struct _UI_DISPLAY_DRIVER* driver;
	void* ctx;
	u32 index;
	ui_display_info_t* display_info;
	const ui_display_info_mode_t* selected_mode;
} ui_display_t;



typedef struct _UI_DISPLAY_DRIVER{
	const char* name;
	_Bool (*resize)(ui_display_t*);
} ui_display_driver_t;



void ui_display_init(void);



ui_display_t* ui_display_add(const ui_display_driver_t* driver,void* ctx,u32 index,const u8* edid,u32 edid_length);



#endif
