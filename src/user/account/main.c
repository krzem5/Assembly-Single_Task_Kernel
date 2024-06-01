#include <account/account.h>
#include <sys/error/error.h>
#include <sys/id/group.h>
#include <sys/id/user.h>
#include <sys/io/io.h>
#include <sys/string/string.h>
#include <sys/types.h>



// 1.
// 2. list
// 3. create user <name>
// 4. create user <name> <password>
// 5. create user <name> <password> <uid>
// 6. create group <name>
// 7. create group <name> <gid>
// 8. switch <name>
// 9. switch <name> <password>



static int _list_users_and_groups(void){
	sys_io_print("Users:\n");
	for (sys_uid_t uid=account_iter_user_start();uid;uid=account_iter_user_next(uid)){
		account_user_t data;
		char buffer[256];
		if (SYS_IS_ERROR(account_get_user_data(uid,&data))||SYS_IS_ERROR(sys_uid_get_name(uid,buffer,sizeof(buffer)))){
			continue;
		}
		sys_io_print("\x1b[1m%u(%s)\x1b[0m:\t%s\n",uid,buffer,((data.flags&ACCOUNT_USER_FLAG_HAS_PASSWORD)?"(has password)":""));
		for (sys_gid_t gid=account_iter_user_group_start(uid);gid;gid=account_iter_user_group_next(uid,gid)){
			if (SYS_IS_ERROR(sys_gid_get_name(gid,buffer,sizeof(buffer)))){
				continue;
			}
			sys_io_print("  \x1b[1m%u(%s)\x1b[0m\n",gid,buffer);
		}
	}
	sys_io_print("Groups:\n");
	for (sys_gid_t gid=account_iter_group_start();gid;gid=account_iter_group_next(gid)){
		account_group_t data;
		char buffer[256];
		if (SYS_IS_ERROR(account_get_group_data(gid,&data))||SYS_IS_ERROR(sys_gid_get_name(gid,buffer,sizeof(buffer)))){
			continue;
		}
		sys_io_print("\x1b[1m%u(%s)\x1b[0m\n",gid,buffer);
	}
	return 0;
}



static int _create_user(const char* name,const char* password,sys_uid_t uid){
	sys_error_t new_uid=account_create_user(uid,name,password,sys_string_length(password));
	if (!SYS_IS_ERROR(new_uid)){
		sys_io_print("Created user '%s' with uid=%u\n",name,new_uid);
		return 0;
	}
	sys_io_print("Error creating user\n");
	return 1;
}



static int _create_group(const char* name,sys_gid_t gid){
	sys_error_t new_gid=account_create_group(gid,name);
	if (!SYS_IS_ERROR(new_gid)){
		sys_io_print("Created group '%s' with gid=%u\n",name,new_gid);
		return 0;
	}
	sys_io_print("Error creating group\n");
	return 1;
}



static int _switch_user(const char* name,const char* password){
	return 1;
}



int main(int argc,const char** argv){
	if (argc<=1||!sys_string_compare(argv[1],"list")){
		return _list_users_and_groups();
	}
	if (!sys_string_compare(argv[1],"create")){
		if (argc<4){
			goto _invalid_arguments;
		}
		if (!sys_string_compare(argv[2],"user")){
			if (argc==4){
				return _create_user(argv[3],"",0);
			}
			if (argc==5){
				return _create_user(argv[3],argv[4],0);
			}
			if (argc==6){
				sys_io_print("Parse UID\n");
			}
			goto _invalid_arguments;
		}
		if (!sys_string_compare(argv[2],"group")){
			if (argc==4){
				return _create_group(argv[3],0);
			}
			if (argc==5){
				sys_io_print("Parse GID\n");
			}
			goto _invalid_arguments;
		}
		goto _invalid_arguments;
	}
	if (!sys_string_compare(argv[1],"switch")){
		if (argc<3){
			goto _invalid_arguments;
		}
		if (argc==3){
			return _switch_user(argv[2],"");
		}
		if (argc==4){
			return _switch_user(argv[2],argv[3]);
		}
		goto _invalid_arguments;
	}
_invalid_arguments:
	sys_io_print("Invalid arguments\n");
	return 1;
}
