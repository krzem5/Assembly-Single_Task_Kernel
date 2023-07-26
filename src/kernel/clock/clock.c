#include <kernel/io/io.h>
#include <kernel/log/log.h>
#include <kernel/types.h>



#define PIT_FREQUENCY 1193182



u64 clock_cpu_frequency;
u64 clock_conversion_factor;
u32 clock_conversion_shift;



static inline u64 _get_tsc(void){
	u32 low;
	u32 high;
	asm volatile("rdtsc":"=a"(low),"=d"(high));
	return (((u64)high)<<32)|low;
}



static inline u64 _wait_for_pit(u8 high){
	for (u32 i=0;i<500000;i++){
		io_port_in8(0x42);
		if (io_port_in8(0x42)<high){
			return (i>10?_get_tsc():0);
		}
	}
	return 0;
}



static u64 _get_cpu_frequency(void){
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



static void _calculate_clock_conversion(void){
	for (clock_conversion_shift=32;clock_conversion_shift;clock_conversion_shift--){
		clock_conversion_factor=((1000000000ull<<clock_conversion_shift)+(clock_cpu_frequency>>1))/clock_cpu_frequency;
		if (!(clock_conversion_factor>>32)){
			return;
		}
	}
	ERROR("Unable to calculate clock conversion factor");
	for (;;);
}



void clock_init(void){
	LOG("Initializing clock source...");
	INFO("Calculating clock frequency...");
	clock_cpu_frequency=_get_cpu_frequency();
	LOG("CPU clock frequency: %lu Hz",clock_cpu_frequency);
	_calculate_clock_conversion();
}
