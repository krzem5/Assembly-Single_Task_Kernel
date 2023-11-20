#include <sys/fd.h>
#include <sys/io.h>
#include <sys/options.h>



typedef struct _LOCK_TYPE_DATA{
	char location[48];
} lock_type_data_t;



static _Bool _read_file(s64 fd,const char* name,char* buffer,u32 buffer_length){
	if (!buffer_length){
		return 0;
	}
	fd=sys_fd_open(fd,name,SYS_FD_FLAG_READ);
	if (fd<=0){
		return 0;
	}
	s64 size=sys_fd_read(fd,buffer,buffer_length-1,0);
	sys_fd_close(fd);
	if (size>0){
		buffer[size]=0;
	}
	return size>0;
}



int main(int argc,const char** argv){
	if (!sys_options_parse(argc,argv,NULL)){
		return 1;
	}
	s64 root_fd=sys_fd_open(0,"/lock",0);
	if (!root_fd){
		printf("lockinfo: unable to open lockfs\n");
		return 1;
	}
	s64 types_fd=sys_fd_open(root_fd,"types",0);
	u32 max_type=0;
	for (s64 iter=sys_fd_iter_start(types_fd);iter>=0;iter=sys_fd_iter_next(iter)){
		char name[32];
		if (sys_fd_iter_get(iter,name,32)<=0){
			continue;
		}
		u32 i=sys_options_atoi(name);
		if (i>max_type){
			max_type=i;
		}
	}
	lock_type_data_t types[max_type+1];
	for (u32 i=0;i<=max_type;i++){
		types[i].location[0]=0;
	}
	for (s64 iter=sys_fd_iter_start(types_fd);iter>=0;iter=sys_fd_iter_next(iter)){
		char buffer[32];
		if (sys_fd_iter_get(iter,buffer,32)<=0){
			continue;
		}
		u32 i=sys_options_atoi(buffer);
		if (!i||i>max_type){
			continue;
		}
		s64 fd=sys_fd_open(types_fd,buffer,0);
		if (fd<=0){
			continue;
		}
		_read_file(fd,"location",(types+i)->location,48);
		sys_fd_close(fd);
		printf("%u: %s\n",i,(types+i)->location);
	}
	sys_fd_close(types_fd);
	s64 data_fd=sys_fd_open(root_fd,"data",0);
	for (s64 iter=sys_fd_iter_start(data_fd);iter>=0;iter=sys_fd_iter_next(iter)){
		char buffer[32];
		if (sys_fd_iter_get(iter,buffer,32)<=0){
			continue;
		}
		s64 fd=sys_fd_open(data_fd,buffer,0);
		if (fd<=0){
			continue;
		}
		char location[48];
		if (!_read_file(fd,"location",location,48)){
			sys_fd_close(fd);
			continue;
		}
		for (s64 iter=sys_fd_iter_start(fd);iter>=0;iter=sys_fd_iter_next(iter)){
			if (sys_fd_iter_get(iter,buffer,32)<=0||buffer[0]<48||buffer[0]>57){
				continue;
			}
			u32 type=sys_options_atoi(buffer);
			if (type>max_type){
				continue;
			}
			printf("%s:%s\n",location,buffer);
		}
		sys_fd_close(fd);
	}
	sys_fd_close(data_fd);
	sys_fd_close(root_fd);
	return 0;
}
