#include <sys/syscall/kernel_syscalls.h>
#include <sys/time/time.h>
#include <sys/types.h>



SYS_PUBLIC u64 sys_time_get_boot_offset(void){
	return _sys_syscall_time_get_boot_offset();
}



// Based on https://howardhinnant.github.io/date_algorithms.html#civil_from_days
SYS_PUBLIC void sys_time_from_nanoseconds(s64 time,sys_time_t* out){
	out->nanoseconds=time%1000;
	time/=1000;
	out->microseconds=time%1000;
	time/=1000;
	out->miliseconds=time%1000;
	time/=1000;
	out->seconds=time%60;
	time/=60;
	out->minutes=time%60;
	time/=60;
	out->hours=time%24;
	time/=24;
	time+=719468;
	u32 era=(time>=0?time:time-146096)/146097;
	u32 day_of_era=time-era*146097;
	u32 year_of_era=(day_of_era-day_of_era/1460+day_of_era/36524-day_of_era/146096)/365;
	u32 day_of_year=day_of_era-(365*year_of_era+year_of_era/4-year_of_era/100);
	u32 month=(5*day_of_year+2)/153;
	out->days=day_of_year-(153*month+2)/5+1;
	out->months=(month<10?month+3:month-9);
	out->years=year_of_era+era*400+(out->months<=2);
}
