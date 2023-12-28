#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <ui/framebuffer.h>
#define KERNEL_LOG_NAME "ui_framebuffer"



static pmm_counter_descriptor_t* _ui_framebuffer_pmm_counter=NULL;
static omm_allocator_t* _ui_framebuffer_allocator=NULL;



void ui_framebuffer_init(void){
	LOG("Initializing UI framebuffers...");
	_ui_framebuffer_pmm_counter=pmm_alloc_counter("ui_framebuffer");
	_ui_framebuffer_allocator=omm_init("ui_framebuffer",sizeof(ui_framebuffer_t),8,2,pmm_alloc_counter("omm_ui_framebuffer"));
	spinlock_init(&(_ui_framebuffer_allocator->lock));
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
	out->data=(void*)(raw_data+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	out->address=raw_data;
	out->size=size;
	out->width=width;
	out->height=height;
	out->format=format;
	return out;
}



KERNEL_PUBLIC void ui_framebuffer_delete(ui_framebuffer_t* fb){
	pmm_dealloc(fb->address,fb->size>>PAGE_SIZE_SHIFT,_ui_framebuffer_pmm_counter);
	omm_dealloc(_ui_framebuffer_allocator,fb);
}
