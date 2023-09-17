#include <kernel/apic/lapic.h>
#include <kernel/clock/clock.h>
#include <kernel/cpu/cpu.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/msr/msr.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "lapic"



#define TIMER_CALIBRATION_TICKS 0xfffff



#define REGISTER_TPR 0x20
#define REGISTER_EOI 0x2c
#define REGISTER_SVR 0x3c
#define REGISTER_ESR 0xa0
#define REGISTER_ICR0 0xc0
#define REGISTER_ICR1 0xc4
#define REGISTER_LVT_TMR 0xc8
#define REGISTER_TMRINITCNT 0xe0
#define REGISTER_TMRCURRCNT 0xe4
#define REGISTER_TMRDIV 0xf8

#define REGISTER_MAX REGISTER_TMRDIV



static u32* _lapic_timer_frequencies;

volatile u32* _lapic_registers;



void lapic_init(u64 base,u16 cpu_count){
	LOG("Initializing lAPIC controller...");
	INFO("lAPIC base: %p",base);
	_lapic_registers=(void*)vmm_identity_map(base,(REGISTER_MAX+1)*sizeof(u32));
	INFO("Enabling APIC...");
	msr_enable_apic();
	_lapic_timer_frequencies=kmm_alloc(cpu_count*sizeof(u32));
}



void lapic_send_ipi(u8 lapic_id,u16 vector){
	_lapic_registers[REGISTER_ESR]=0;
	_lapic_registers[REGISTER_ESR]=0;
	_lapic_registers[REGISTER_ICR1]=lapic_id<<24;
	_lapic_registers[REGISTER_ICR0]=vector;
	SPINLOOP(_lapic_registers[REGISTER_ICR0]&APIC_ICR0_DELIVERY_STATUS_PENDING);
}



void lapic_enable(void){
	_lapic_registers[REGISTER_SVR]=0x100|LAPIC_SPURIOUS_VECTOR;
	_lapic_registers[REGISTER_ESR]=0;
	_lapic_registers[REGISTER_ESR]=0;
	_lapic_registers[REGISTER_TPR]=0;
	LOG("Calibrating lAPIC timer...");
	_lapic_registers[REGISTER_TMRINITCNT]=0;
	_lapic_registers[REGISTER_LVT_TMR]=0x00010000|LAPIC_SPURIOUS_VECTOR;
	_lapic_registers[REGISTER_TMRDIV]=0;
	u64 start_time=clock_get_time();
	_lapic_registers[REGISTER_TMRINITCNT]=TIMER_CALIBRATION_TICKS;
	SPINLOOP(_lapic_registers[REGISTER_TMRCURRCNT]);
	u64 end_time=clock_get_time();
	_lapic_registers[REGISTER_TMRINITCNT]=0;
	_lapic_registers[REGISTER_LVT_TMR]=LAPIC_DISABLE_TIMER;
	_lapic_timer_frequencies[CPU_HEADER_DATA->cpu_data->index]=TIMER_CALIBRATION_TICKS/((end_time-start_time+500)/1000);
	INFO("Timer frequency: %u ticks/us",_lapic_timer_frequencies[CPU_HEADER_DATA->cpu_data->index]);
}



void lapic_timer_start(u32 time_us){
	_lapic_registers[REGISTER_EOI]=0;
	_lapic_registers[REGISTER_LVT_TMR]=LAPIC_SCHEDULER_VECTOR;
	_lapic_registers[REGISTER_TMRDIV]=0;
	_lapic_registers[REGISTER_TMRINITCNT]=time_us*_lapic_timer_frequencies[CPU_HEADER_DATA->cpu_data->index];
}



u32 lapic_timer_stop(void){
	u32 out=_lapic_registers[REGISTER_TMRCURRCNT];
	_lapic_registers[REGISTER_TMRINITCNT]=0;
	_lapic_registers[REGISTER_LVT_TMR]=LAPIC_DISABLE_TIMER;
	return (out?out/_lapic_timer_frequencies[CPU_HEADER_DATA->cpu_data->index]:0);
}
