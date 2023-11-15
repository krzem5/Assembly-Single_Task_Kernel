#ifndef _COMMAND_H_
#define _COMMAND_H_ 1
#include <sys/types.h>



#define MAX_ARG_COUNT 64

#define DECLARE_COMMAND(name,help) \
	static const command_t _command_##name={ \
		#name, \
		name##_main, \
		help \
	}; \
	static const command_t* __attribute__((used,section("commands"))) _command_ptr_##name=&_command_##name;



typedef struct _COMMAND{
	const char* name;
	void (*func)(int,const char*const*);
	const char* help;
} command_t;



void command_execute(const char* command);



#endif
