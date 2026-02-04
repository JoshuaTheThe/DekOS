#include <drivers/sys/rtc.h>

static uint8_t bcd2Bin(uint8_t bcd)
{
        return (bcd & 0x0F) + ((bcd >> 4) * 10);
}

static uint8_t rtcRead(uint8_t reg)
{
        outb(RTC_ADDRESS_PORT, reg);
        return inb(RTC_DATA_PORT);
}

rtcTime_t rtcGetTime(void)
{
        rtcTime_t time = {0};
        time.second = bcd2Bin(rtcRead(RTC_SECONDS));
        time.minute = bcd2Bin(rtcRead(RTC_MINUTES));
        time.hour = bcd2Bin(rtcRead(RTC_HOURS));
        time.day = bcd2Bin(rtcRead(RTC_DAY));
        time.month = bcd2Bin(rtcRead(RTC_MONTH));
        time.year = bcd2Bin(rtcRead(RTC_YEAR));
        return time;
}

