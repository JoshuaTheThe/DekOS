#ifndef RTC
#define RTC

#include <utils.h>
#include <io.h>

#define RTC_ADDRESS_PORT 0x70
#define RTC_DATA_PORT 0x71

#define RTC_SECONDS 0x00
#define RTC_MINUTES 0x02
#define RTC_HOURS 0x04
#define RTC_DAY 0x07
#define RTC_MONTH 0x08
#define RTC_YEAR 0x09
#define BASE_YEAR (2000)

typedef struct
{
        uint8_t day, month, year;
        uint8_t hour, minute, second;
} rtcTime_t;

rtcTime_t rtcGetTime(void);

#endif
