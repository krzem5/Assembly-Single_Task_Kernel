#include <GL/gl.h>
#include <opengl/opengl.h>
#include <sys/error/error.h>
#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/mp/thread.h>
#include <sys/mp/timer.h>
#include <sys/types.h>
#include <ui/display.h>



static const char*const _ui_framebuffer_format_names[UI_DISPLAY_FRAMEBUFFER_FORMAT_MAX+1]={
	[UI_DISPLAY_FRAMEBUFFER_FORMAT_BGRX]="BGRX",
	[UI_DISPLAY_FRAMEBUFFER_FORMAT_RGBX]="RGBX",
	[UI_DISPLAY_FRAMEBUFFER_FORMAT_XBGR]="XBGR",
	[UI_DISPLAY_FRAMEBUFFER_FORMAT_XRGB]="XRGB",
};



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
		if (SYS_IS_ERROR(ui_display_get_data(display,&data))){
			continue;
		}
		sys_io_print("Display #%u: %u x %u @ %u Hz\n",data.index,data.mode.width,data.mode.height,data.mode.freq);
		ui_display_info_t temp_info;
		if (SYS_IS_ERROR(ui_display_get_info(display,&temp_info,sizeof(ui_display_info_t)))){
			continue;
		}
		ui_display_info_t* info=sys_heap_alloc(NULL,sizeof(ui_display_info_t)+temp_info.mode_count*sizeof(ui_display_mode_t));
		if (SYS_IS_ERROR(ui_display_get_info(display,info,sizeof(ui_display_info_t)+temp_info.mode_count*sizeof(ui_display_mode_t)))){
			continue;
		}
		sys_io_print("Modes: (%u)\n",temp_info.mode_count);
		for (u32 i=0;i<temp_info.mode_count;i++){
			sys_io_print("  %u x %u @ %u Hz\n",info->modes[i].width,info->modes[i].height,info->modes[i].freq);
		}
		sys_heap_dealloc(NULL,info);
		ui_framebuffer_handle_t framebuffer=ui_display_get_display_framebuffer(display);
		ui_display_framebuffer_t config;
		ui_display_get_framebuffer_config(framebuffer,&config);
		u32* framebuffer_address=(u32*)ui_display_map_framebuffer(framebuffer);
		sys_io_print("Framebuffer: %v, %u x %u, %s -> %p\n",config.size,config.width,config.height,_ui_framebuffer_format_names[config.format],framebuffer_address);
		opengl_set_state_framebuffer(state,framebuffer);
		opengl_set_state(state);
		glViewport(0,0,config.width,config.height);
		sys_io_print("GL_RENDERER: %s\n",glGetString(GL_RENDERER));
		sys_io_print("GL_SHADING_LANGUAGE_VERSION: %s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
		sys_io_print("GL_VENDOR: %s\n",glGetString(GL_VENDOR));
		sys_io_print("GL_VERSION: %s\n",glGetString(GL_VERSION));
		GLint extension_count;
		glGetIntegerv(GL_NUM_EXTENSIONS,&extension_count);
		sys_io_print("GL_NUM_EXTENSIONS: %u\n",extension_count);
		sys_event_t timer_event=sys_timer_get_event(sys_timer_create(1000000000ull/data.mode.freq,0xffffffffffffffffull));
		u32 t=0;
		while (1){
			u8 color[3];
			_hsl_to_rgb(t*255/120,127,255,color);
			glClearColor(color[0]/255.0f,color[1]/255.0f,color[2]/255.0f,1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glFlush();
			ui_display_flush_display_framebuffer(display);
			t++;
			sys_thread_await_events(&timer_event,1);
		}
	}
	return 0;
}
