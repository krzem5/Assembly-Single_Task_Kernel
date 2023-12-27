#include <kernel/module/module.h>
#include <virgl/virgl.h>
#include <virtio/gpu.h>



static _Bool _init(module_t* module){
	virgl_init();
	virtio_gpu_init();
	return 1;
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	_init,
	_deinit,
	0
);
