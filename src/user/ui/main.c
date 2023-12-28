#include <opengl/opengl.h>
#include <sys/error.h>
#include <sys/io.h>
#include <ui/display.h>



int main(int argc,const char** argv){
	opengl_init();
	opengl_create_state(330);
	for (ui_display_handle_t display=ui_display_iter_start();display;display=ui_display_iter_next(display)){
		ui_display_data_t data;
		if (SYS_ERROR_IS_ERROR(ui_display_get_data(display,&data))){
			continue;
		}
		printf("Display %p: #%u, %u x %u @ %u Hz\n",display,data.index,data.mode.width,data.mode.height,data.mode.freq);
	}
	return 0;
}
