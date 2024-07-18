#include <kernel/bios/bios.h>
#include <kernel/io/io.h>
#include <kernel/module/module.h>
#include <kernel/shutdown/shutdown.h>
#include <kernel/util/string.h>



static void _qemu_shutdown_function(void){
	io_port_out16(0xb004,0x2000);
}



MODULE_INIT(){
	if (bios_data.manufacturer->length!=4||!str_equal(bios_data.manufacturer->data,"QEMU")){
		module_unload(module_self);
		return;
	}
	shutdown_register_shutdown_function(_qemu_shutdown_function,1);
}



MODULE_DECLARE(0);
