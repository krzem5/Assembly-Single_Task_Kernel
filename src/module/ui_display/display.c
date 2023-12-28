#include <kernel/handle/handle.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <ui/display.h>
#include <ui/display_info.h>
#include <ui/framebuffer.h>
#define KERNEL_LOG_NAME "ui_display"



static omm_allocator_t* _ui_display_allocator=NULL;

handle_type_t ui_display_handle_type;



void ui_display_init(void){
	LOG("Initializing UI displays...");
	_ui_display_allocator=omm_init("ui_display",sizeof(ui_display_t),8,2,pmm_alloc_counter("omm_ui_display"));
	spinlock_init(&(_ui_display_allocator->lock));
	ui_display_handle_type=handle_alloc("ui_display",NULL);
}



KERNEL_PUBLIC ui_display_t* ui_display_create(const ui_display_driver_t* driver,void* ctx,u32 index,const u8* edid,u32 edid_length){
	ui_display_info_t* info=ui_display_info_parse_edid(edid,edid_length);
	if (!info){
		return NULL;
	}
	ui_display_t* out=omm_alloc(_ui_display_allocator);
	handle_new(out,ui_display_handle_type,&(out->handle));
	out->driver=driver;
	out->ctx=ctx;
	out->index=index;
	out->display_info=info;
	out->mode=NULL;
	out->framebuffer=NULL;
	handle_finish_setup(&(out->handle));
	return out;
}



KERNEL_PUBLIC _Bool ui_display_set_mode(ui_display_t* display,const ui_display_info_mode_t* mode){
	display->mode=mode;
	return display->driver->resize_framebuffer(display);
}
