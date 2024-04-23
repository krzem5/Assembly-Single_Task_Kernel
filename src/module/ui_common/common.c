#include <kernel/handle/handle.h>
#include <kernel/types.h>



static handle_id_t _ui_common_process=0;



KERNEL_PUBLIC handle_id_t ui_common_get_process(void){
	return _ui_common_process;
}



KERNEL_PUBLIC bool ui_common_set_process(handle_id_t process_handle){
	if (_ui_common_process){
		return 0;
	}
	_ui_common_process=process_handle;
	return 1;
}
