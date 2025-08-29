#include <rtc.h>

uint8_t rtc_read(uint8_t reg)
{
        outb(RTC_ADDRESS_PORT, reg);
        return inb(RTC_DATA_PORT);
}

uint8_t bcd_to_bin(uint8_t bcd)
{
        return (bcd & 0x0F) + ((bcd >> 4) * 10);
}

void rtc_get_time(uint8_t *hour, uint8_t *minute, uint8_t *second)
{
        *second = bcd_to_bin(rtc_read(RTC_SECONDS));
        *minute = bcd_to_bin(rtc_read(RTC_MINUTES));
        *hour = bcd_to_bin(rtc_read(RTC_HOURS));
}

void rtc_get_date(uint8_t *day, uint8_t *month, uint8_t *year)
{
        *day = bcd_to_bin(rtc_read(RTC_DAY));
        *month = bcd_to_bin(rtc_read(RTC_MONTH));
        *year = bcd_to_bin(rtc_read(RTC_YEAR));
}
