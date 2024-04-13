#include <kernel/module/module.h>
#include <kfs2/fs.h>
#include <kfs2/io.h>



MODULE_INIT(){
	kfs2_io_init();
	kfs2_register_fs();
}



MODULE_DECLARE(
	NULL,
	NULL,
	0
);
