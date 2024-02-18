#include <kernel/module/module.h>
#include <kernel/types.h>



void aaa(void){
	return;
}



extern void bbb(void);



void zzz(void){
	bbb();
}



static _Bool _init(module_t* module){
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

