#ifndef _COMMAND_H_
#define _COMMAND_H_ 1
#include <user/types.h>



#define MAX_ARG_COUNT 64


#define DECLARE_COMMAND(name) \
	static const command_t _command_##name={ \
		#name, \
		name##_main \
	}; \
	static const command_t* __attribute__((used,section("commands"))) _command_ptr_##name=&_command_##name;



typedef struct _COMMAND{
	const char* name;
	void (*func)(int,const char*const*);
} command_t;



void command_execute(const char* command);



#endif
