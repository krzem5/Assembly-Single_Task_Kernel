#ifndef _COMMAND_H_
#define _COMMAND_H_ 1
#include <user/types.h>



#define MAX_ARG_COUNT 64

#define OPTION_TYPE_SWITCH 0
#define OPTION_TYPE_INT 1
#define OPTION_TYPE_STRING 2
#define OPTION_TYPE_ARG 3
#define OPTION_TYPE_END 255


#define DECLARE_COMMAND(name,help,...) \
	static const option_t _command_options_##name[]={ \
		__VA_ARGS__ \
		{OPTION_TYPE_END} \
	}; \
	static const command_t _command_##name={ \
		#name, \
		name##_main, \
		help, \
		_command_options_##name, \
	}; \
	static const command_t* __attribute__((used,section("commands"))) _command_ptr_##name=&_command_##name;



typedef struct _OPTION{
	u8 type;
	char short_name;
	const char* name;
} option_t;



typedef struct _COMMAND{
	const char* name;
	void (*func)(int,const char*const*);
	const char* help;
	const option_t* options;
} command_t;



void command_execute(const char* command);



#endif
