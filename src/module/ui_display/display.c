#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <ui/display.h>
#define KERNEL_LOG_NAME "ui_display"



void ui_display_init(void){
	LOG("Initializing UI displays...");
}



KERNEL_PUBLIC ui_display_t* ui_add_display(const ui_display_driver_t* driver,void* ctx,u32 index,const u8* edid,u32 edid_length){
	LOG("Parsing EDID...");
	if ((edid[0]|edid[7])||(edid[1]&edid[2]&edid[3]&edid[4]&edid[5]&edid[6])!=0xff){
		WARN("Invalid EDID header");
		return NULL;
	}
	u16 id=(edid[8]<<8)|edid[9];
	WARN("%c%c%c",((id>>10)&0x1f)+64,((id>>5)&0x1f)+64,((id>>0)&0x1f)+64);
	// panic("A");
	return NULL;
}
