#include <sys/acl/acl.h>
#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/io/io.h>
#include <sys/types.h>
#include <sys/util/options.h>



#define FD_ACL_FLAG_STAT 1
#define FD_ACL_FLAG_DUP 2
#define FD_ACL_FLAG_IO 4



int main(int argc,const char** argv){
	if (!sys_options_parse(argc,argv,NULL)){
		return 1;
	}
	sys_fd_t fd=sys_fd_open(0,"/",0);
	sys_acl_set_permissions(fd,0,0,FD_ACL_FLAG_STAT|FD_ACL_FLAG_DUP|FD_ACL_FLAG_IO);
	sys_acl_request_permissions(fd,0,FD_ACL_FLAG_STAT|FD_ACL_FLAG_DUP);
	sys_acl_request_permissions(fd,0,FD_ACL_FLAG_IO);
	return 0;
}
