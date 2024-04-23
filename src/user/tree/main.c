#include <dircolor/dircolor.h>
#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/io/io.h>
#include <sys/types.h>
#include <sys/util/options.h>



// Must be a multiple of 64
#define MAX_LEVELS 1024



typedef struct _FRAME{
	u64 bitmap[MAX_LEVELS>>6];
	u32 file_count;
	u32 directory_count;
} frame_t;



static void _list_files(sys_fd_t fd,u32 level,frame_t* frame){
	if (level>=MAX_LEVELS){
		return;
	}
	for (sys_fd_iterator_t iter=sys_fd_iter_start(fd);!SYS_IS_ERROR(iter);){
		char name[256];
		if (SYS_IS_ERROR(sys_fd_iter_get(iter,name,256))){
			iter=sys_fd_iter_next(iter);
			continue;
		}
		sys_fd_t child=sys_fd_open(fd,name,SYS_FD_FLAG_READ|SYS_FD_FLAG_IGNORE_LINKS);
		if (SYS_IS_ERROR(child)){
			iter=sys_fd_iter_next(iter);
			continue;
		}
		sys_fd_stat_t stat;
		if (SYS_IS_ERROR(sys_fd_stat(child,&stat))){
			sys_fd_close(child);
			iter=sys_fd_iter_next(iter);
			continue;
		}
		iter=sys_fd_iter_next(iter);
		bool has_next_sibling=!SYS_IS_ERROR(iter);
		for (u32 i=0;i<level;i++){
			sys_io_print("%s   ",((frame->bitmap[i>>6]&(1ull<<(i&63)))?"│":" "));
		}
		sys_io_print("%s── ",(has_next_sibling?"├":"└"));
		dircolor_get_color_with_link(&stat,stat.name,child);
		sys_io_print("\n");
		if (stat.type==SYS_FD_STAT_TYPE_DIRECTORY){
			frame->directory_count++;
			u64 mask=1ull<<(level&63);
			if (has_next_sibling){
				frame->bitmap[level>>6]|=mask;
			}
			else{
				frame->bitmap[level>>6]&=~mask;
			}
			_list_files(child,level+1,frame);
		}
		else{
			frame->file_count++;
		}
		sys_fd_close(child);
	}
}



int main(int argc,const char** argv){
	u32 i=sys_options_parse(argc,argv,NULL);
	if (!i){
		return 1;
	}
	const char* directory=(i<argc?argv[i]:".");
	sys_fd_t fd=sys_fd_open(0,directory,0);
	if (SYS_IS_ERROR(fd)){
		sys_io_print("tree: unable to open file '%s': error %d\n",directory,fd);
		return 1;
	}
	sys_fd_stat_t stat;
	if (SYS_IS_ERROR(sys_fd_stat(fd,&stat))){
		sys_io_print("tree: unable to stat file '%s'\n",directory);
		return 1;
	}
	char prefix[32];
	dircolor_get_color(&stat,prefix);
	sys_io_print("%s%s\x1b[0m\n",prefix,directory);
	frame_t frame={
		.file_count=0,
		.directory_count=0
	};
	for (u32 i=0;i<(MAX_LEVELS>>6);i++){
		frame.bitmap[i]=0;
	}
	_list_files(fd,0,&frame);
	sys_fd_close(fd);
	sys_io_print("\n%lu director%s, %lu file%s\n",frame.directory_count,(frame.directory_count==1?"y":"ies"),frame.file_count,(frame.file_count==1?"":"s"));
	return 0;
}
