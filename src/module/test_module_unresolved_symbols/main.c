#include <kernel/module/module.h>
#include <kernel/types.h>



void aaa(void){
	return;
}



extern void bbb(void);



void zzz(void){
	bbb();
}



MODULE_DECLARE_NEW(0);
