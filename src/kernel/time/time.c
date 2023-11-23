#include <kernel/clock/clock.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/time/time.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "time"



KERNEL_PUBLIC u64 KERNEL_INIT_WRITE time_boot_offset;



void KERNEL_EARLY_EXEC time_init(void){
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
