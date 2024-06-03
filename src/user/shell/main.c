#include <input.h>
#include <shell/interpreter.h>



void main(void){
	while (1){
		shell_interpreter_execute(input_get());
	}
}
