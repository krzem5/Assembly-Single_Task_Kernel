#include <shell/input.h>
#include <shell/interpreter.h>



void main(void){
	while (1){
		interpreter_execute(input_get());
	}
}
