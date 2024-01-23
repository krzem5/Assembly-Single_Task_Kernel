#ifndef _UI_FRAMEBUFFER_H_
#define _UI_FRAMEBUFFER_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/types.h>



#define UI_FRAMEBUFFER_FORMAT_NONE 0
#define UI_FRAMEBUFFER_FORMAT_BGRX 1
#define UI_FRAMEBUFFER_FORMAT_RGBX 2
#define UI_FRAMEBUFFER_FORMAT_XBGR 3
#define UI_FRAMEBUFFER_FORMAT_XRGB 4

#define UI_FRAMEBUFFER_FORMAT_MIN UI_FRAMEBUFFER_FORMAT_BGRX
#define UI_FRAMEBUFFER_FORMAT_MAX UI_FRAMEBUFFER_FORMAT_XRGB

#define UI_FRAMEBUFFER_ACL_FLAG_MAP 1



struct _UI_DISPLAY;



typedef struct _UI_FRAMEBUFFER{
	handle_t handle;
	struct _UI_DISPLAY* display;
	u32* data;
	u64 address;
	u64 size;
	u32 width;
	u32 height;
	u32 format;
} ui_framebuffer_t;



extern handle_type_t ui_framebuffer_handle_type;



void ui_framebuffer_init(void);



ui_framebuffer_t* ui_framebuffer_create(struct _UI_DISPLAY* display,u32 width,u32 height,u32 format);



void ui_framebuffer_delete(ui_framebuffer_t* fb);



#endif
