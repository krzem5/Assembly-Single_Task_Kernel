#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <ui/display.h>
#include <ui/display_info.h>
#define KERNEL_LOG_NAME "ui_display"



void ui_display_init(void){
	LOG("Initializing UI displays...");
}



KERNEL_PUBLIC ui_display_t* ui_display_add(const ui_display_driver_t* driver,void* ctx,u32 index,const u8* edid,u32 edid_length){
	ui_display_info_t* info=ui_display_info_parse_edid(edid,edid_length);
	if (!info){
		return NULL;
	}
	// panic("A");
	return NULL;
}
