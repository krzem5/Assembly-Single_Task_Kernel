#include <account/account.h>
#include <sys/error/error.h>
#include <sys/id/group.h>
#include <sys/id/user.h>
#include <sys/io/io.h>
#include <sys/string/string.h>
#include <sys/types.h>



int main(int argc,const char** argv){
	sys_io_print("Users:\n");
	for (sys_uid_t uid=account_iter_user_start();uid;uid=account_iter_user_next(uid)){
		account_user_t data;
		char buffer[256];
		if (SYS_IS_ERROR(account_get_user_data(uid,&data))||SYS_IS_ERROR(sys_uid_get_name(uid,buffer,sizeof(buffer)))){
			continue;
		}
		sys_io_print("\x1b[1m%u\x1b[0m:\t%s\t%s\n",uid,buffer,((data.flags&ACCOUNT_USER_FLAG_HAS_PASSWORD)?"(has password)":""));
	}
	sys_io_print("Groups:\n");
	for (sys_gid_t gid=account_iter_group_start();gid;gid=account_iter_group_next(gid)){
		account_group_t data;
		char buffer[256];
		if (SYS_IS_ERROR(account_get_group_data(gid,&data))||SYS_IS_ERROR(sys_gid_get_name(gid,buffer,sizeof(buffer)))){
			continue;
		}
		sys_io_print("\x1b[1m%u\x1b[0m:\t%s\n",gid,buffer);
	}
	return 0;
}
