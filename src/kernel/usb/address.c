#include <kernel/types.h>
#include <kernel/usb/address.h>
#include <kernel/util/util.h>



void usb_address_space_init(usb_address_space_t* address_space){
	address_space->data[0]=0xfffffffffffffffeull;
	address_space->data[1]=0xffffffffffffffffull;
}



u8 usb_address_space_alloc(usb_address_space_t* address_space){
	if (address_space->data[0]){
		u8 out=__builtin_ffsll(address_space->data[0])-1;
		address_space->data[0]&=address_space->data[0]-1;
		return out;
	}
	else if (address_space->data[1]){
		u8 out=__builtin_ffsll(address_space->data[1])-1;
		address_space->data[1]&=address_space->data[1]-1;
		return out+64;
	}
	else{
		panic("USB address space exhausted");
	}
}



void usb_address_space_dealloc(usb_address_space_t* address_space,u8 address){
	if (address){
		address_space->data[address>>6]|=1ull<<address;
	}
}
