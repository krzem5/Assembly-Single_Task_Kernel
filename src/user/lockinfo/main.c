#include <sys2/clock/clock.h>
#include <sys2/error/error.h>
#include <sys2/fd/fd.h>
#include <sys2/io/io.h>
#include <sys2/types.h>
#include <sys2/util/options.h>



typedef struct _LOCK_TYPE_DATA{
	char location[48];
	char name[48];
} lock_type_data_t;



static _Bool _read_file(sys2_fd_t fd,const char* name,char* buffer,u32 buffer_length){
	if (!buffer_length){
		return 0;
	}
	fd=sys2_fd_open(fd,name,SYS2_FD_FLAG_READ);
	if (SYS2_IS_ERROR(fd)){
		return 0;
	}
	u64 size=sys2_fd_read(fd,buffer,buffer_length-1,0);
	sys2_fd_close(fd);
	if (!SYS2_IS_ERROR(size)){
		buffer[size]=0;
	}
	return !SYS2_IS_ERROR(size);
}



int main(int argc,const char** argv){
	if (!sys2_options_parse(argc,argv,NULL)){
		return 1;
	}
	sys2_fd_t root_fd=sys2_fd_open(0,"/lock",0);
	if (SYS2_IS_ERROR(root_fd)){
		sys2_io_print("lockinfo: unable to open lockfs\n");
		return 1;
	}
	sys2_fd_t types_fd=sys2_fd_open(root_fd,"type",0);
	u32 max_type=0;
	for (sys2_fd_iterator_t iter=sys2_fd_iter_start(types_fd);!SYS2_IS_ERROR(iter);iter=sys2_fd_iter_next(iter)){
		char name[32];
		if (SYS2_IS_ERROR(sys2_fd_iter_get(iter,name,32))){
			continue;
		}
		u32 i=sys2_options_atoi(name);
		if (i>max_type){
			max_type=i;
		}
	}
	lock_type_data_t types[max_type+1];
	for (u32 i=0;i<=max_type;i++){
		types[i].location[0]=0;
		types[i].name[0]=0;
	}
	for (sys2_fd_iterator_t iter=sys2_fd_iter_start(types_fd);!SYS2_IS_ERROR(iter);iter=sys2_fd_iter_next(iter)){
		char buffer[32];
		if (SYS2_IS_ERROR(sys2_fd_iter_get(iter,buffer,32))){
			continue;
		}
		u32 i=sys2_options_atoi(buffer);
		if (!i||i>max_type){
			continue;
		}
		sys2_fd_t fd=sys2_fd_open(types_fd,buffer,0);
		if (SYS2_IS_ERROR(fd)){
			continue;
		}
		_read_file(fd,"location",(types+i)->location,48);
		_read_file(fd,"name",(types+i)->name,48);
		sys2_fd_close(fd);
	}
	sys2_fd_close(types_fd);
	sys2_fd_t data_fd=sys2_fd_open(root_fd,"data",0);
	for (sys2_fd_iterator_t iter=sys2_fd_iter_start(data_fd);!SYS2_IS_ERROR(iter);iter=sys2_fd_iter_next(iter)){
		char buffer[32];
		if (SYS2_IS_ERROR(sys2_fd_iter_get(iter,buffer,32))){
			continue;
		}
		sys2_fd_t fd=sys2_fd_open(data_fd,buffer,0);
		if (SYS2_IS_ERROR(fd)){
			continue;
		}
		char location[48];
		if (!_read_file(fd,"location",location,48)){
			sys2_fd_close(fd);
			continue;
		}
		char name[48];
		if (!_read_file(fd,"name",name,48)){
			sys2_fd_close(fd);
			continue;
		}
		_Bool header_printed=0;
		for (sys2_fd_iterator_t iter_inner=sys2_fd_iter_start(fd);!SYS2_IS_ERROR(iter_inner);iter_inner=sys2_fd_iter_next(iter_inner)){
			if (SYS2_IS_ERROR(sys2_fd_iter_get(iter_inner,buffer,32))||buffer[0]<48||buffer[0]>57){
				continue;
			}
			u32 type=sys2_options_atoi(buffer);
			if (type>max_type){
				continue;
			}
			sys2_fd_t subfd=sys2_fd_open(fd,buffer,0);
			if (SYS2_IS_ERROR(subfd)){
				continue;
			}
			if (!_read_file(subfd,"count",buffer,32)){
				goto _cleanup_subfd;
			}
			u32 count=sys2_options_atoi(buffer);
			if (!count||!_read_file(subfd,"ticks",buffer,32)){
				goto _cleanup_subfd;
			}
			u32 ticks=sys2_options_atoi(buffer);
			if (!_read_file(subfd,"max_ticks",buffer,32)){
				goto _cleanup_subfd;
			}
			u32 max_ticks=sys2_options_atoi(buffer);
			if (!header_printed){
				sys2_io_print("%s \x1b[2;3m%s\x1b[0m\n",location,name);
				header_printed=1;
			}
			if (type){
				sys2_io_print("    %s \x1b[2;3m%s\x1b[0m\n",(types+type)->location,(types+type)->name);
			}
			else{
				sys2_io_print("    %s\n",(types+type)->location);
			}
			sys2_io_print("        cnt: \x1b[1m%lu\x1b[0m\n        avg: \x1b[1m%lu\x1b[0m ns\n        max: \x1b[1m%lu\x1b[0m ns\n",count,sys2_clock_convert_ticks_to_time(ticks/count),sys2_clock_convert_ticks_to_time(max_ticks));
_cleanup_subfd:
			sys2_fd_close(subfd);
		}
		sys2_fd_close(fd);
	}
	sys2_fd_close(data_fd);
	sys2_fd_close(root_fd);
	return 0;
}
