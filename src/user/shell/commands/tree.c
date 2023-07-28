#include <command.h>
#include <cwd.h>
#include <string.h>
#include <user/fs.h>
#include <user/io.h>



// Must be a multiple of 64
#define MAX_LEVELS 1024



typedef struct _FRAME{
	u64 bitmap[MAX_LEVELS>>6];
	u32 file_count;
	u32 directory_count;
} frame_t;



static void _list_files(int fd,u32 level,frame_t* frame){
	if (level>=MAX_LEVELS){
		return;
	}
	int child=fs_get_relative(fd,FS_RELATIVE_FIRST_CHILD,0);
	fs_stat_t stat;
	while (child>=0){
		if (fs_stat(child,&stat)<0){
			fs_close(child);
			break;
		}
		int next_child=fs_get_relative(child,FS_RELATIVE_NEXT_SIBLING,0);
		_Bool has_next_sibling=(next_child>=0);
		fs_close(next_child);
		for (u32 i=0;i<level;i++){
			printf("%s   ",((frame->bitmap[i>>6]&(1ull<<(i&63)))?"│":" "));
		}
		printf("%s── %s\n",(has_next_sibling?"├":"└"),stat.name);
		if (stat.type==FS_STAT_TYPE_DIRECTORY){
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
		next_child=fs_get_relative(child,FS_RELATIVE_NEXT_SIBLING,0);
		fs_close(child);
		child=next_child;
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
		int fd=fs_open(cwd_fd,directory,0);
		if (fd<0){
			printf("tree: unable to open file '%s': error %d\n",directory,fd);
			return;
		}
		printf("%s\n",directory);
		_list_files(fd,0,&frame);
		fs_close(fd);
	}
	printf("\n%lu directorie%s, %lu file%s\n",frame.directory_count,(frame.directory_count==1?"":"s"),frame.file_count,(frame.file_count==1?"":"s"));
}



DECLARE_COMMAND(tree,"tree [<directory>]");
