#include <gpt/partition.h>
#include <kernel/module/module.h>



static _Bool _init(module_t* module){
	gpt_register_partition_table();
	return 1;
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	"gpt",
	_init,
	_deinit
);
