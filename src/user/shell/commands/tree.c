#include <command.h>
#include <cwd.h>
#include <dircolor/dircolor.h>
#include <string.h>
#include <sys/fd.h>
#include <sys/io.h>
#include <sys/memory.h>
#include <sys/types.h>



// Must be a multiple of 64
#define MAX_LEVELS 1024



typedef struct _FRAME{
	u64 bitmap[MAX_LEVELS>>6];
	u32 file_count;
	u32 directory_count;
} frame_t;



static void _list_files(s64 fd,u32 level,frame_t* frame){
	if (level>=MAX_LEVELS){
		return;
	}
	for (s64 iter=sys_fd_iter_start(fd);iter>=0;){
		char name[256];
		if (sys_fd_iter_get(iter,name,256)<=0){
			iter=sys_fd_iter_next(iter);
			continue;
		}
		s64 child=sys_fd_open(fd,name,SYS_FD_FLAG_READ|SYS_FD_FLAG_IGNORE_LINKS);
		if (child<0){
			iter=sys_fd_iter_next(iter);
			continue;
		}
		sys_fd_stat_t stat;
		if (sys_fd_stat(child,&stat)<0){
			sys_fd_close(child);
			iter=sys_fd_iter_next(iter);
			continue;
		}
		iter=sys_fd_iter_next(iter);
		_Bool has_next_sibling=(iter>=0);
		for (u32 i=0;i<level;i++){
			printf("%s   ",((frame->bitmap[i>>6]&(1ull<<(i&63)))?"│":" "));
		}
		printf("%s── ",(has_next_sibling?"├":"└"));
		dircolor_get_color_with_link(&stat,stat.name,child);
		putchar('\n');
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



void tree_main(int argc,const char*const* argv){
	const char* directory=NULL;
	for (u32 i=1;i<argc;i++){
		if (argv[i][0]!='-'&&!directory){
			directory=argv[i];
		}
		else{
			printf("tree: unrecognized option '%s'\n",argv[i]);
			return;
		}
	}
	s64 fd=(directory?sys_fd_open(cwd_fd,directory,0):cwd_fd);
	if (fd<0){
		printf("tree: unable to open file '%s': error %d\n",directory,fd);
		return;
	}
	sys_fd_stat_t stat;
	if (sys_fd_stat(fd,&stat)<0){
		printf("tree: unable to stat file '%s'\n",(directory?directory:"."));
		return;
	}
	char prefix[32];
	dircolor_get_color(&stat,prefix);
	printf("%s%s\x1b[0m\n",prefix,(directory?directory:"."));
	frame_t frame={
		.file_count=0,
		.directory_count=0
	};
	for (u32 i=0;i<(MAX_LEVELS>>6);i++){
		frame.bitmap[i]=0;
	}
	_list_files(fd,0,&frame);
	if (fd!=cwd_fd){
		sys_fd_close(fd);
	}
	printf("\n%lu director%s, %lu file%s\n",frame.directory_count,(frame.directory_count==1?"y":"ies"),frame.file_count,(frame.file_count==1?"":"s"));
}



DECLARE_COMMAND(tree,"tree [<directory>]");
