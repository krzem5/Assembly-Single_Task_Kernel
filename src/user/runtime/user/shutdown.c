#include <user/syscall.h>



void shutdown(_Bool restart){
	_syscall_acpi_shutdown(restart);
}
