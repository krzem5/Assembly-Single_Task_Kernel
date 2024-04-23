#include <common/kfs2/api.h>
#include <common/kfs2/structures.h>
#include <common/types.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>



static void* _drive_data=NULL;
static u64 _drive_block_size=0;



static u64 _current_time(void){
	struct timespec tm;
	clock_gettime(CLOCK_REALTIME,&tm);
	return tm.tv_sec*1000000000+tm.tv_nsec;
}



static u64 _read_callback(void* ctx,u64 offset,void* buffer,u64 size){
	memcpy(buffer,_drive_data+offset*_drive_block_size,size*_drive_block_size);
	return size;
}



static u64 _write_callback(void* ctx,u64 offset,const void* buffer,u64 size){
	memcpy(_drive_data+offset*_drive_block_size,buffer,size*_drive_block_size);
	return size;
}



static void* _alloc_callback(u64 count){
	return calloc(1,count<<12);
}



static void _dealloc_callback(void* ptr,u64 count){
	free(ptr);
}



static bool _lookup_path(kfs2_filesystem_t* fs,const char* path,kfs2_node_t* parent,const char** child_name,kfs2_node_t* out){
	kfs2_filesystem_get_root(fs,out);
	if (child_name){
		*child_name=NULL;
	}
	while (path[0]){
		if (path[0]=='/'){
			path++;
			continue;
		}
		u64 i=0;
		for (;path[i]&&path[i]!='/';i++){
			if (i>255){
				return 0;
			}
		}
		if (i==1&&path[0]=='.'){
			path+=1;
			continue;
		}
		if (i==2&&path[0]=='.'&&path[1]=='.'){
			printf("Backtracking is not supported\n");
			return 0;
		}
		kfs2_node_t child;
		if (!kfs2_node_lookup(fs,out,path,i,&child)){
			if (child_name&&!path[i]){
				*parent=*out;
				*child_name=path;
			}
			return 0;
		}
		*out=child;
		path+=i;
	}
	return 1;
}



static bool _command_format(const kfs2_filesystem_config_t* fs_config){
	kfs2_filesystem_t fs;
	if (!kfs2_filesystem_format(fs_config,&fs)){
		printf("Unable to format partition as KFS2\n");
		return 1;
	}
	srand((u32)time(NULL));
	for (u32 i=0;i<16;i++){
		fs.root_block.uuid[i]=rand();
	}
	fs.root_block.uuid[6]=(fs.root_block.uuid[6]&0x0f)|0x40;
	fs.root_block.uuid[8]=(fs.root_block.uuid[8]&0x3f)|0x80;
	kfs2_filesystem_flush_root_block(&fs);
	kfs2_node_t root_node;
	kfs2_filesystem_get_root(&fs,&root_node);
	root_node.flags=(root_node.flags&(~KFS2_INODE_PERMISSION_MASK))|(0755<<KFS2_INODE_PERMISSION_SHIFT);
	root_node.time_access=_current_time();
	root_node.time_modify=root_node.time_access;
	root_node.time_change=root_node.time_access;
	root_node.time_birth=root_node.time_access;
	kfs2_node_flush(&fs,&root_node);
	kfs2_filesystem_deinit(&fs);
	return 0;
}



static bool _command_mkdir(kfs2_filesystem_t* fs,int argc,const char** argv){
	if (argc<6){
		printf("Usage:\n\n%s <drive file> <block_size>:<start_lba>:<end_lba> mkdir <path> <permissions>\n",(argc?argv[0]:"kfs2"));
		return 1;
	}
	u32 permissions;
	if (sscanf(argv[5],"%i",&permissions)<0||(permissions>>9)){
		printf("Invalid permissions flags '%s'\n",argv[5]);
		return 1;
	}
	kfs2_node_t parent;
	const char* child_name;
	kfs2_node_t node;
	if (!_lookup_path(fs,argv[4],&parent,&child_name,&node)){
		if (!child_name){
			printf("Path '%s' is not valid\n",argv[4]);
			return 1;
		}
		if (!kfs2_node_create(fs,&parent,child_name,strlen(child_name),KFS2_INODE_TYPE_DIRECTORY,&node)){
			printf("Unable to create directory '%s'\n",argv[4]);
		}
		node.time_access=_current_time();
		node.time_modify=node.time_access;
		node.time_change=node.time_access;
		node.time_birth=node.time_access;
	}
	node.flags=(node.flags&(~KFS2_INODE_PERMISSION_MASK))|(permissions<<KFS2_INODE_PERMISSION_SHIFT);
	node.time_modify=_current_time();
	node.time_change=node.time_modify;
	kfs2_node_flush(fs,&node);
	return 0;
}



static bool _command_ls(kfs2_filesystem_t* fs,int argc,const char** argv){
	if (argc<5){
		printf("Usage:\n\n%s <drive file> <block_size>:<start_lba>:<end_lba> ls <path>\n",(argc?argv[0]:"kfs2"));
		return 1;
	}
	kfs2_node_t node;
	if (!_lookup_path(fs,argv[4],NULL,NULL,&node)){
		printf("File '%s' not found\n",argv[4]);
		return 1;
	}
	if ((node.flags&KFS2_INODE_TYPE_MASK)!=KFS2_INODE_TYPE_DIRECTORY){
		printf("File '%s' is not a directory\n",argv[4]);
		return 1;
	}
	char buffer[256];
	u64 pointer=0;
	while (1){
		u32 buffer_length=sizeof(buffer)-1;
		pointer=kfs2_node_iterate(fs,&node,pointer,buffer,&buffer_length);
		if (!pointer){
			break;
		}
		buffer[buffer_length]=0;
		printf("%s\n",buffer);
	}
	return 0;
}



static bool _command_copy(kfs2_filesystem_t* fs,int argc,const char** argv){
	if (argc<7){
		printf("Usage:\n\n%s <drive file> <block_size>:<start_lba>:<end_lba> copy <path> <permissions> <host_path>\n",(argc?argv[0]:"kfs2"));
		return 1;
	}
	u32 permissions;
	if (sscanf(argv[5],"%i",&permissions)<0||(permissions>>9)){
		printf("Invalid permissions flags '%s'\n",argv[5]);
		return 1;
	}
	kfs2_node_t parent;
	const char* child_name;
	kfs2_node_t node;
	if (!_lookup_path(fs,argv[4],&parent,&child_name,&node)){
		if (!child_name){
			printf("Path '%s' is not valid\n",argv[4]);
			return 1;
		}
		if (!kfs2_node_create(fs,&parent,child_name,strlen(child_name),KFS2_INODE_TYPE_FILE,&node)){
			printf("Unable to create file '%s'\n",argv[4]);
		}
		node.time_access=_current_time();
		node.time_modify=node.time_access;
		node.time_change=node.time_access;
		node.time_birth=node.time_access;
		kfs2_node_flush(fs,&node);
	}
	if ((node.flags&KFS2_INODE_TYPE_MASK)!=KFS2_INODE_TYPE_FILE){
		printf("Path '%s' is not a file\n",argv[4]);
		return 1;
	}
	node.flags=(node.flags&(~KFS2_INODE_PERMISSION_MASK))|(permissions<<KFS2_INODE_PERMISSION_SHIFT);
	node.time_modify=_current_time();
	node.time_change=node.time_modify;
	kfs2_node_flush(fs,&node);
	int fd=open(argv[6],O_RDONLY,0);
	if (fd<0){
		printf("Unable to open host file '%s'\n",argv[6]);
		return 1;
	}
	u64 file_size=lseek(fd,0,SEEK_END);
	lseek(fd,0,SEEK_SET);
	kfs2_node_resize(fs,&node,file_size);
	void* file_data=mmap(0,file_size,PROT_READ,MAP_SHARED,fd,0);
	close(fd);
	if (kfs2_node_write(fs,&node,0,file_data,file_size)!=file_size){
		munmap(file_data,file_size);
		printf("Unable to write data to file\n");
		return 1;
	}
	munmap(file_data,file_size);
	return 0;
}



static bool _command_link(kfs2_filesystem_t* fs,int argc,const char** argv){
	if (argc<7){
		printf("Usage:\n\n%s <drive file> <block_size>:<start_lba>:<end_lba> link <path> <permissions> <target>\n",(argc?argv[0]:"kfs2"));
		return 1;
	}
	u32 permissions;
	if (sscanf(argv[5],"%i",&permissions)<0||(permissions>>9)){
		printf("Invalid permissions flags '%s'\n",argv[5]);
		return 1;
	}
	kfs2_node_t parent;
	const char* child_name;
	kfs2_node_t node;
	if (!_lookup_path(fs,argv[4],&parent,&child_name,&node)){
		if (!child_name){
			printf("Path '%s' is not valid\n",argv[4]);
			return 1;
		}
		if (!kfs2_node_create(fs,&parent,child_name,strlen(child_name),KFS2_INODE_TYPE_LINK,&node)){
			printf("Unable to create link '%s'\n",argv[4]);
		}
		node.time_access=_current_time();
		node.time_modify=node.time_access;
		node.time_change=node.time_access;
		node.time_birth=node.time_access;
		kfs2_node_flush(fs,&node);
	}
	if ((node.flags&KFS2_INODE_TYPE_MASK)!=KFS2_INODE_TYPE_LINK){
		printf("Path '%s' is not a link\n",argv[4]);
		return 1;
	}
	node.flags=(node.flags&(~KFS2_INODE_PERMISSION_MASK))|(permissions<<KFS2_INODE_PERMISSION_SHIFT);
	node.time_modify=_current_time();
	node.time_change=node.time_modify;
	kfs2_node_flush(fs,&node);
	u64 target_length=strlen(argv[6]);
	kfs2_node_resize(fs,&node,target_length);
	if (kfs2_node_write(fs,&node,0,argv[6],target_length)!=target_length){
		printf("Unable to write data to link\n");
		return 1;
	}
	return 0;
}



int main(int argc,const char** argv){
	if (argc<4){
		printf("Usage:\n\n%s <drive file> <block_size>:<start_lba>:<end_lba> <command> [...arguments]\n",(argc?argv[0]:"kfs2"));
		return 1;
	}
	int fd=open(argv[1],O_RDWR,0);
	if (fd<0){
		printf("Unable to open drive file '%s'\n",argv[1]);
		return 1;
	}
	u64 drive_size=lseek(fd,0,SEEK_END);
	lseek(fd,0,SEEK_SET);
	u64 start_lba;
	u64 end_lba;
	if (sscanf(argv[2],"%llu:%llu:%llu",&_drive_block_size,&start_lba,&end_lba)<0||(_drive_block_size&(_drive_block_size-1))||end_lba<=start_lba||end_lba*_drive_block_size>drive_size){
		printf("Invalid partition description '%s'\n",argv[2]);
		return 1;
	}
	_drive_data=mmap(0,drive_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	close(fd);
	int ret=1;
	const kfs2_filesystem_config_t fs_config={
		NULL,
		(kfs2_filesystem_block_read_callback_t)_read_callback,
		(kfs2_filesystem_block_write_callback_t)_write_callback,
		(kfs2_filesystem_page_alloc_callback_t)_alloc_callback,
		(kfs2_filesystem_page_dealloc_callback_t)_dealloc_callback,
		_drive_block_size,
		start_lba,
		end_lba
	};
	if (!strcmp(argv[3],"format")){
		ret=_command_format(&fs_config);
		goto _cleanup_without_fs;
	}
	kfs2_filesystem_t fs;
	if (!kfs2_filesystem_init(&fs_config,&fs)){
		printf("KFS2 corrupted\n");
		goto _cleanup_without_fs;
	}
	if (!strcmp(argv[3],"mkdir")){
		ret=_command_mkdir(&fs,argc,argv);
	}
	else if (!strcmp(argv[3],"ls")){
		ret=_command_ls(&fs,argc,argv);
	}
	else if (!strcmp(argv[3],"copy")){
		ret=_command_copy(&fs,argc,argv);
	}
	else if (!strcmp(argv[3],"link")){
		ret=_command_link(&fs,argc,argv);
	}
	else{
		printf("Unknown command '%s'\n",argv[3]);
	}
	kfs2_filesystem_deinit(&fs);
_cleanup_without_fs:
	munmap(_drive_data,drive_size);
	return ret;
}
