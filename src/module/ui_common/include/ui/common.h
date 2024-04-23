#ifndef _UI_COMMON_H_
#define _UI_COMMON_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/types.h>



handle_id_t ui_common_get_process(void);



bool ui_common_set_process(handle_id_t process_handle);



#endif
