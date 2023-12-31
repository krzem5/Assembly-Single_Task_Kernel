#include <GL/gl.h>
#include <opengl/opengl.h>
#include <sys/_kernel_syscall.h>
#include <sys/error.h>
#include <sys/io.h>
#include <ui/display.h>



static void _hsl_to_rgb(u8 h,u8 s,u8 l,u8* rgb){
	if (!s){
		rgb[0]=l;
		rgb[1]=l;
		rgb[2]=l;
		return;
	}
	u8 r=h/43;
	u8 m=(h-(r*43))*6;
	u8 p=(l*(255-s))>>8;
	u8 q=(l*(255-((s*m)>>8)))>>8;
	u8 t=(l*(255-((s*(255-m))>>8)))>>8;
	switch (r){
		case 0:
			rgb[0]=l;
			rgb[1]=t;
			rgb[2]=p;
			break;
		case 1:
			rgb[0]=q;
			rgb[1]=l;
			rgb[2]=p;
			break;
		case 2:
			rgb[0]=p;
			rgb[1]=l;
			rgb[2]=t;
			break;
		case 3:
			rgb[0]=p;
			rgb[1]=q;
			rgb[2]=l;
			break;
		case 4:
			rgb[0]=t;
			rgb[1]=p;
			rgb[2]=l;
			break;
		default:
			rgb[0]=l;
			rgb[1]=p;
			rgb[2]=q;
			break;
	}
}



int main(int argc,const char** argv){
	opengl_init();
	opengl_state_t state=opengl_create_state(330);
	for (ui_display_handle_t display=ui_display_iter_start();display;display=ui_display_iter_next(display)){
		ui_display_data_t data;
		if (SYS_ERROR_IS_ERROR(ui_display_get_data(display,&data))){
			continue;
		}
		printf("Display #%u: %u x %u @ %u Hz\n",data.index,data.mode.width,data.mode.height,data.mode.freq);
		ui_framebuffer_handle_t framebuffer=ui_display_get_display_framebuffer(display);
		ui_display_framebuffer_t config;
		ui_display_get_framebuffer_config(framebuffer,&config);
		u32* framebuffer_address=(u32*)ui_display_map_framebuffer(framebuffer);
		printf("Framebuffer: %v, %u x %u, [%u] -> %p\n",config.size,config.width,config.height,config.format,framebuffer_address);
		opengl_set_state_framebuffer(state,framebuffer);
		opengl_set_state(state);
		glViewport(0,0,config.width,config.height);
		printf("GL_RENDERER: %s\n",glGetString(GL_RENDERER));
		printf("GL_SHADING_LANGUAGE_VERSION: %s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
		printf("GL_VENDOR: %s\n",glGetString(GL_VENDOR));
		printf("GL_VERSION: %s\n",glGetString(GL_VERSION));
		GLint extension_count;
		glGetIntegerv(GL_NUM_EXTENSIONS,&extension_count);
		printf("GL_NUM_EXTENSIONS: %u\n",extension_count);
		u64 timer_event=_syscall_timer_get_event(_syscall_timer_create(1000000000ull/data.mode.freq,0xffffffffffffffffull));
		u32 t=0;
		while (1){
			u8 color[3];
			_hsl_to_rgb(t*255/120,127,255,color);
			glClearColor(color[0]/255.0f,color[1]/255.0f,color[2]/255.0f,1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glFlush();
			ui_display_flush_display_framebuffer(display);
			t++;
			_syscall_thread_await_events(&timer_event,1);
		}
	}
	return 0;
}
