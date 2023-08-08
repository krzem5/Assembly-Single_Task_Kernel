#ifndef _KERNEL_NETWORK_SYSCALL_H_
#define _KERNEL_NETWORK_SYSCALL_H_ 1
#include <kernel/syscall/syscall.h>



void syscall_network_layer1_config(syscall_registers_t* regs);



void syscall_network_layer2_send(syscall_registers_t* regs);



void syscall_network_layer2_poll(syscall_registers_t* regs);



void syscall_network_layer3_refresh(syscall_registers_t* regs);



void syscall_network_layer3_device_count(syscall_registers_t* regs);



void syscall_network_layer3_device_get(syscall_registers_t* regs);



#endif
