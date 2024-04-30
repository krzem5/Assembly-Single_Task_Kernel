#include <kernel/acl/acl.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/module/module.h>
#include <kernel/mp/thread.h>
#include <kernel/types.h>
#include <ui/display.h>
#include <ui/framebuffer.h>
#define KERNEL_LOG_NAME "ui_framebuffer"



static pmm_counter_descriptor_t* KERNEL_INIT_WRITE _ui_framebuffer_pmm_counter=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _ui_framebuffer_allocator=NULL;

KERNEL_PUBLIC handle_type_t KERNEL_INIT_WRITE ui_framebuffer_handle_type=0;



MODULE_INIT(){
	LOG("Initializing UI framebuffers...");
	_ui_framebuffer_pmm_counter=pmm_alloc_counter("ui_framebuffer");
	_ui_framebuffer_allocator=omm_init("ui_framebuffer",sizeof(ui_framebuffer_t),8,2);
	rwlock_init(&(_ui_framebuffer_allocator->lock));
	ui_framebuffer_handle_type=handle_alloc("ui_framebuffer",NULL);
}



KERNEL_PUBLIC ui_framebuffer_t* ui_framebuffer_create(ui_display_t* display,u32 width,u32 height,u32 format){
	if (format<UI_FRAMEBUFFER_FORMAT_MIN||format>UI_FRAMEBUFFER_FORMAT_MAX){
		ERROR("Invalid framebuffer format");
		return NULL;
	}
	ui_framebuffer_t* out=omm_alloc(_ui_framebuffer_allocator);
	handle_new(out,ui_framebuffer_handle_type,&(out->handle));
	out->handle.acl=acl_create();
	acl_set(out->handle.acl,THREAD_DATA->process,0,UI_FRAMEBUFFER_ACL_FLAG_USE);
	out->display=display;
	out->gpu_handle=0;
	out->width=width;
	out->height=height;
	out->format=format;
	if (display->driver->create_framebuffer(out)){
		handle_finish_setup(&(out->handle));
		return out;
	}
	ERROR("Unable to create framebuffer");
	handle_finish_setup(&(out->handle));
	handle_release(&(out->handle));
	omm_dealloc(_ui_framebuffer_allocator,out);
	return NULL;
}



KERNEL_PUBLIC void ui_framebuffer_delete(ui_framebuffer_t* fb){
	fb->display->driver->delete_framebuffer(fb);
	omm_dealloc(_ui_framebuffer_allocator,fb);
}
