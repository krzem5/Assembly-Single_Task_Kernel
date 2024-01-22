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

static const char* _vertex_shader=" \
#version 330 core \n\
 \n\
 \n\
 \n\
layout (location=0) in vec2 in_pos; \n\
out vec4 fs_color; \n\
 \n\
 \n\
 \n\
void main(void){ \n\
	gl_Position=vec4(in_pos,0.0,1.0); \n\
	fs_color=vec4(1.0,0.0,0.0,0.0); \n\
} \n\
";

static const char* _fragment_shader=" \
#version 330 core \n\
 \n\
 \n\
 \n\
in vec4 fs_color; \n\
out vec4 out_color; \n\
 \n\
 \n\
 \n\
void main(void){ \n\
	out_color=fs_color; \n\
} \n\
";



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
		opengl_state_t state=opengl_create_state(330);
		opengl_set_state_framebuffer(state,framebuffer);
		opengl_set_state(state);
		sys_io_print("GL_RENDERER: %s\n",glGetString(GL_RENDERER));
		sys_io_print("GL_SHADING_LANGUAGE_VERSION: %s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
		sys_io_print("GL_VENDOR: %s\n",glGetString(GL_VENDOR));
		sys_io_print("GL_VERSION: %s\n",glGetString(GL_VERSION));
		GLint extension_count;
		glGetIntegerv(GL_NUM_EXTENSIONS,&extension_count);
		sys_io_print("GL_NUM_EXTENSIONS: %u\n",extension_count);
		{
			GLuint vertex_shader=glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex_shader,1,&_vertex_shader,NULL);
			glCompileShader(vertex_shader);
			GLint compilation_status;
			glGetShaderiv(vertex_shader,GL_COMPILE_STATUS,&compilation_status);
			if (compilation_status!=GL_TRUE){
				GLsizei length;
				glGetShaderiv(vertex_shader,GL_INFO_LOG_LENGTH,&length);
				char* buffer=sys_heap_alloc(NULL,length);
				glGetShaderInfoLog(vertex_shader,length,&length,buffer);
				sys_io_print("%s\n",buffer);
				sys_heap_dealloc(NULL,buffer);
				return 1;
			}
			GLuint fragment_shader=glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment_shader,1,&_fragment_shader,NULL);
			glCompileShader(fragment_shader);
			glGetShaderiv(fragment_shader,GL_COMPILE_STATUS,&compilation_status);
			if (compilation_status!=GL_TRUE){
				GLsizei length;
				glGetShaderiv(fragment_shader,GL_INFO_LOG_LENGTH,&length);
				char* buffer=sys_heap_alloc(NULL,length);
				glGetShaderInfoLog(fragment_shader,length,&length,buffer);
				sys_io_print("%s\n",buffer);
				sys_heap_dealloc(NULL,buffer);
				return 1;
			}
			GLuint program=glCreateProgram();
			glAttachShader(program,vertex_shader);
			glAttachShader(program,fragment_shader);
			glLinkProgram(program);
			GLint link_status;
			glGetProgramiv(program,GL_LINK_STATUS,&link_status);
			if (link_status!=GL_TRUE){
				GLsizei length;
				glGetProgramiv(program,GL_INFO_LOG_LENGTH,&length);
				char* buffer=sys_heap_alloc(NULL,length);
				glGetProgramInfoLog(program,length,&length,buffer);
				sys_io_print("%s\n",buffer);
				sys_heap_dealloc(NULL,buffer);
				return 1;
			}
			glUseProgram(program);
		}
		sys_event_t timer_event=sys_timer_get_event(sys_timer_create(1000000000ull/data.mode.freq,SYS_TIMER_COUNT_INFINITE));
		u32 t=0;
		glViewport(0,0,config.width,config.height);
		while (1){
			u8 color[3];
			_hsl_to_rgb(t*255/120,127,255,color);
			glClearColor(color[0]/255.0f,color[1]/255.0f,color[2]/255.0f,1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glFlush();
			ui_display_flush_display_framebuffer(display);
			t++;
			sys_thread_await_events(&timer_event,1);
			break; /***********************************************/
		}
	}
	return 0;
}
