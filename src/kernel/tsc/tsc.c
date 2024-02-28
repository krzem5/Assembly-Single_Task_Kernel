#include <kernel/clock/clock.h>
#include <kernel/io/io.h>
#include <kernel/types.h>



#define PIT_FREQUENCY 1193182



static _Bool KERNEL_INIT_WRITE _tsc_pit_clock_source_is_stable=0;



static KERNEL_INLINE u64 KERNEL_EARLY_EXEC _wait_for_pit(u8 high){
	for (u32 i=0;i<500000;i++){
		io_port_in8(0x42);
		if (io_port_in8(0x42)<high){
			if (i<10){
				return 0;
			}
			u32 low;
			u32 high;
			asm volatile("rdtsc":"=a"(low),"=d"(high));
			return (((u64)high)<<32)|low;
		}
	}
	return 0;
}



static u64 KERNEL_EARLY_EXEC _calibration_callback(void){
	while (1){
		io_port_out8(0x61,(io_port_in8(0x61)&0xfd)|0x01);
		io_port_out8(0x43,0xb0);
		io_port_out8(0x42,0xff);
		io_port_out8(0x42,0xff);
		io_port_in8(0x42);
		io_port_in8(0x42);
		u64 start_tsc=_wait_for_pit(255);
		if (!start_tsc){
			continue;
		}
		u16 start_pit=io_port_in8(0x42)|(io_port_in8(0x42)<<8);
		for (u8 i=128;i<255;i++){
			u64 end_tsc=_wait_for_pit(255-i);
			if (!end_tsc){
				continue;
			}
			u16 end_pit=io_port_in8(0x42)|(io_port_in8(0x42)<<8);
			if ((end_pit>>8)!=254-i){
				continue;
			}
			return (end_tsc-start_tsc)*PIT_FREQUENCY/(start_pit-end_pit);
		}
	}
}



static const clock_source_t _tsc_pit_clock_source={
	"TSC/PIT",
	_calibration_callback,
	&_tsc_pit_clock_source_is_stable
};



KERNEL_EARLY_EARLY_INIT(){
	clock_add_source(&_tsc_pit_clock_source);
}
