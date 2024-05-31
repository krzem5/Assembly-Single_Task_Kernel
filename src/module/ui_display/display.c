#include <kernel/handle/handle.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <ui/display.h>
#include <ui/display_info.h>
#include <ui/framebuffer.h>
#define KERNEL_LOG_NAME "ui_display"



static omm_allocator_t* KERNEL_INIT_WRITE _ui_display_allocator=NULL;

KERNEL_PUBLIC handle_type_t KERNEL_INIT_WRITE ui_display_handle_type=0;



MODULE_INIT(){
	LOG("Initializing UI displays...");
	_ui_display_allocator=omm_init("ui.display",sizeof(ui_display_t),8,2);
	rwlock_init(&(_ui_display_allocator->lock));
	ui_display_handle_type=handle_alloc("ui.display",NULL);
}



KERNEL_PUBLIC ui_display_t* ui_display_create(const ui_display_driver_t* driver,void* ctx,u32 index,const u8* edid,u32 edid_length){
	ui_display_info_t* info=ui_display_info_parse_edid(edid,edid_length);
	if (!info){
		return NULL;
	}
	ui_display_t* out=omm_alloc(_ui_display_allocator);
	handle_new(ui_display_handle_type,&(out->handle));
	out->driver=driver;
	out->ctx=ctx;
	out->index=index;
	out->display_info=info;
	out->mode=NULL;
	out->framebuffer=NULL;
	return out;
}



KERNEL_PUBLIC bool ui_display_set_mode(ui_display_t* display,const ui_display_info_mode_t* mode){
	display->mode=mode;
	return display->driver->resize_framebuffer(display);
}
