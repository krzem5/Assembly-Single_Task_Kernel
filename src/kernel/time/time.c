#include <kernel/clock/clock.h>
#include <kernel/error/error.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/time/time.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "time"



#define TIME_TYPE_BOOT 0
#define TIME_TYPE_EARLY_INIT 1
#define TIME_TYPE_INIT 2



KERNEL_PUBLIC u64 KERNEL_INIT_WRITE time_boot_offset=0;
KERNEL_PUBLIC u64 time_early_init_offset=0;
KERNEL_PUBLIC u64 time_init_offset=0;



KERNEL_EARLY_EARLY_INIT(){
	LOG("Calculating boot time...");
	time_boot_offset=time_to_nanoseconds(kernel_data.date.year,kernel_data.date.month,kernel_data.date.day,kernel_data.date.hour,kernel_data.date.minute,kernel_data.date.second)+kernel_data.date.nanosecond-clock_ticks_to_time(kernel_data.date.measurement_offset);
	INFO("Boot time offset: %lu",time_boot_offset);
}



// Based on https://howardhinnant.github.io/date_algorithms.html#days_from_civil
KERNEL_PUBLIC s64 time_to_nanoseconds(s16 year,u8 month,u8 day,u8 hour,u8 minute,u8 second){
	year-=(month<=2);
	u32 era=(year>=0?year:year-399)/400;
	u32 year_of_era=year-era*400;
	u32 day_of_year=(153*(month>2?month-3:month+9)+2)/5+day-1;
	u32 day_of_era=year_of_era*365+year_of_era/4-year_of_era/100+day_of_year;
	u64 days=era*146097+day_of_era-719468;
	return (((days*24+hour)*60+minute)*60+second)*1000000000;
}



error_t syscall_time_get(u32 type){
	if (type==TIME_TYPE_BOOT){
		return time_boot_offset;
	}
	if (type==TIME_TYPE_EARLY_INIT){
		return time_early_init_offset;
	}
	if (type==TIME_TYPE_INIT){
		return time_init_offset;
	}
	return ERROR_NOT_FOUND;
}
