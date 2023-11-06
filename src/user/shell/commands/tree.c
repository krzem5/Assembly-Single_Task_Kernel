#include <command.h>
#include <cwd.h>
#include <string.h>
#include <user/fd.h>
#include <user/io.h>
#include <user/types.h>



// Must be a multiple of 64
#define MAX_LEVELS 1024



typedef struct _FRAME{
	u64 bitmap[MAX_LEVELS>>6];
	u32 file_count;
	u32 directory_count;
} frame_t;



static void _print_file_name(const fd_stat_t* stat,const char* name){
	const char* prefix="";
	if (stat->type==FD_STAT_TYPE_DIRECTORY){
		prefix="\x1b[1;34m";
	}
	else if (stat->type==FD_STAT_TYPE_LINK){
		prefix="\x1b[1;36m";
	}
	printf("%s%s\x1b[0m",prefix,name);
}



static void _list_files(s64 fd,u32 level,frame_t* frame){
	if (level>=MAX_LEVELS){
		return;
	}
	for (s64 iter=fd_iter_start(fd);iter>=0;){
		char name[256];
		if (fd_iter_get(iter,name,256)<=0){
			iter=fd_iter_next(iter);
			continue;
		}
		s64 child=fd_open(fd,name,FD_FLAG_READ);
		if (child<0){
			iter=fd_iter_next(iter);
			continue;
		}
		fd_stat_t stat;
		if (fd_stat(child,&stat)<0){
			fd_close(child);
			iter=fd_iter_next(iter);
			continue;
		}
		iter=fd_iter_next(iter);
		_Bool has_next_sibling=(iter>=0);
		for (u32 i=0;i<level;i++){
			printf("%s   ",((frame->bitmap[i>>6]&(1ull<<(i&63)))?"│":" "));
		}
		printf("%s── ",(has_next_sibling?"├":"└"));
		_print_file_name(&stat,stat.name);
		if (stat.type==FD_STAT_TYPE_LINK){
			printf(" -> ");
			char link_buffer[4096];
			if (fd_read(child,link_buffer,4096)<=0){
				printf("???");
			}
			else{
				int link_fd=fd_open(fd,link_buffer,0);
				fd_stat_t link_stat;
				if (link_fd<0||fd_stat(link_fd,&link_stat)<0){
					printf("???");
				}
				else{
					_print_file_name(&link_stat,link_buffer);
				}
				if (link_fd>=0){
					fd_close(link_fd);
				}
			}
		}
		putchar('\n');
		if (stat.type==FD_STAT_TYPE_DIRECTORY){
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
		fd_close(child);
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
	frame_t frame={
		.file_count=0,
		.directory_count=0
	};
	for (u32 i=0;i<(MAX_LEVELS>>6);i++){
		frame.bitmap[i]=0;
	}
	if (!directory){
		printf(".\n");
		_list_files(cwd_fd,0,&frame);
	}
	else{
		s64 fd=fd_open(cwd_fd,directory,0);
		if (fd<0){
			printf("tree: unable to open file '%s': error %d\n",directory,fd);
			return;
		}
		printf("%s\n",directory);
		_list_files(fd,0,&frame);
		fd_close(fd);
	}
	printf("\n%lu director%s, %lu file%s\n",frame.directory_count,(frame.directory_count==1?"y":"ies"),frame.file_count,(frame.file_count==1?"":"s"));
}



DECLARE_COMMAND(tree,"tree [<directory>]");
