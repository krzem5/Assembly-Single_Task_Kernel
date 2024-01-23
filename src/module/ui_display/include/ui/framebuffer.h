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

#define UI_FRAMEBUFFER2_ACL_FLAG_USE 1



struct _UI_DISPLAY;



typedef struct _UI_FRAMEBUFFER2{
	handle_t handle;
	struct _UI_DISPLAY* display;
	u64 gpu_handle;
	u32 width;
	u32 height;
	u32 format;
} ui_framebuffer2_t;



extern handle_type_t ui_framebuffer2_handle_type;



void ui_framebuffer_init(void);



ui_framebuffer2_t* ui_framebuffer2_create(struct _UI_DISPLAY* display,u32 width,u32 height,u32 format);



void ui_framebuffer2_delete(ui_framebuffer2_t* fb);



#endif
