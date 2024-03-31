#include <kernel/bios/bios.h>
#include <kernel/io/io.h>
#include <kernel/module/module.h>
#include <kernel/shutdown/shutdown.h>
#include <kernel/util/string.h>



static void _qemu_shutdown_function(void){
	io_port_out16(0xb004,0x2000);
}



static _Bool _init(module_t* module){
	if (bios_data.manufacturer->length==4&&str_equal(bios_data.manufacturer->data,"QEMU")){
		shutdown_register_shutdown_function(_qemu_shutdown_function,1);
		return 1;
	}
	return 0;
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	_init,
	_deinit,
	0
);
