#include <opengl/opengl.h>
#include <sys/error.h>
#include <sys/io.h>
#include <sys/syscall.h>
#include <ui/display.h>



int main(int argc,const char** argv){
	opengl_init();
	opengl_state_t state=opengl_create_state(330);
	for (ui_display_handle_t display=ui_display_iter_start();display;display=ui_display_iter_next(display)){
		ui_display_data_t data;
		if (SYS_ERROR_IS_ERROR(ui_display_get_data(display,&data))){
			continue;
		}
		printf("Display %p: #%u, %u x %u @ %u Hz\n",display,data.index,data.mode.width,data.mode.height,data.mode.freq);
		ui_framebuffer_handle_t framebuffer=ui_display_get_display_framebuffer(display);
		ui_display_framebuffer_t config;
		ui_display_get_framebuffer_config(framebuffer,&config);
		u32* framebuffer_address=(u32*)ui_display_map_framebuffer(framebuffer);
		printf("Framebuffer: %p, %v, %u x %u, [%u]\n",framebuffer,config.size,config.width,config.height,config.format);
		for (u32 i=0;i<config.width*config.height;i++){
			framebuffer_address[i]=0x000000;
		}
		opengl_set_state_framebuffer(state,framebuffer);
		printf("State: %p\n",state);
		u64 timer_event=_syscall_timer_get_event(_syscall_timer_create(1000000000ull/data.mode.freq,0xffffffffffffffffull));
		u32 t=0;
		while (1){
			_syscall_thread_await_events(&timer_event,1);
			for (u32 i=0;i<config.width*config.height;i++){
				framebuffer_address[i]=i*0x010101-t;
			}
			ui_display_flush_display_framebuffer(display);
			t++;
		}
	}
	return 0;
}
