#include <kernel/acl/acl.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mp/thread.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <ui/framebuffer.h>
#define KERNEL_LOG_NAME "ui_framebuffer"



static pmm_counter_descriptor_t* _ui_framebuffer_pmm_counter=NULL;
static omm_allocator_t* _ui_framebuffer_allocator=NULL;

handle_type_t ui_framebuffer_handle_type=0;



void ui_framebuffer_init(void){
	LOG("Initializing UI framebuffers...");
	_ui_framebuffer_pmm_counter=pmm_alloc_counter("ui_framebuffer");
	_ui_framebuffer_allocator=omm_init("ui_framebuffer",sizeof(ui_framebuffer_t),8,2,pmm_alloc_counter("omm_ui_framebuffer"));
	spinlock_init(&(_ui_framebuffer_allocator->lock));
	ui_framebuffer_handle_type=handle_alloc("ui_framebuffer",NULL);
}



KERNEL_PUBLIC ui_framebuffer_t* ui_framebuffer_create(u32 width,u32 height,u32 format){
	if (format<UI_FRAMEBUFFER_FORMAT_MIN||format>UI_FRAMEBUFFER_FORMAT_MAX){
		ERROR("Invalid framebuffer format");
		return NULL;
	}
	u64 size=pmm_align_up_address(((u64)width)*height*sizeof(u32));
	u64 raw_data=pmm_alloc(size>>PAGE_SIZE_SHIFT,_ui_framebuffer_pmm_counter,0);
	if (!raw_data){
		ERROR("Unable to create framebuffer");
		return NULL;
	}
	ui_framebuffer_t* out=omm_alloc(_ui_framebuffer_allocator);
	handle_new(out,ui_framebuffer_handle_type,&(out->handle));
	out->handle.acl=acl_create();
	acl_set(out->handle.acl,THREAD_DATA->process,0,UI_FRAMEBUFFER_ACL_FLAG_MAP);
	out->data=(void*)(raw_data+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	out->address=raw_data;
	out->size=size;
	out->width=width;
	out->height=height;
	out->format=format;
	handle_finish_setup(&(out->handle));
	return out;
}



KERNEL_PUBLIC void ui_framebuffer_delete(ui_framebuffer_t* fb){
	pmm_dealloc(fb->address,fb->size>>PAGE_SIZE_SHIFT,_ui_framebuffer_pmm_counter);
	omm_dealloc(_ui_framebuffer_allocator,fb);
}
