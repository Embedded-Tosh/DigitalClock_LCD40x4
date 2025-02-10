#include "Arduino.h"
#include <Wire.h>
#include "DS1340lib.h"
#include "digitalClock.h"
#include "time.h"
#include "LCD_40X4.h"

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

RTC_DS1340 RTC;

const static char *WeekDays[] =
    {
        "Sunday",
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday"};

const char *monts[] =
    {
        "Jan",
        "Feb",
        "Mar",
        "Apr",
        "May",
        "Jun",
        "Jul",
        "Aug",
        "Sep",
        "Oct",
        "Nov",
        "Dec"};

void dateTimePrint(DateTime &dt)
{
    Serial.printf("%04d/%02d/%02d %02d:%02d:%02d\n",
                  dt.year(), dt.month(), dt.day(),
                  dt.hour(), dt.minute(), dt.second());
}

void ds1340_RTC_Init(void)
{
    Wire.begin(); // Start the I2C
    RTC.begin();
}

void setDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec)
{
    ds1340_RTC_Init();
    RTC.adjust(DateTime(year, month, day, hour, min, sec)); // Time and date is expanded to date and time on your computer at compiletime
}

void getDateTime(void)
{
    DateTime now = RTC.now(); // Read the time and date from the DS1340

    //dateTimePrint(now);
    displayTime(10, now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year(), (char *)monts[now.month() - 1], (char *)WeekDays[now.dayOfWeek()]);
}

void SyncRTCwithServer(void)
{
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    struct tm timeinfo;
    int attempts = 0;
    while (!getLocalTime(&timeinfo) && attempts < 5)
    {
        Serial.println("Retrying NTP sync...");
        delay(1000);
        attempts++;
    }
    if (attempts == 5)
    {
        Serial.println("Failed to sync with NTP server.");
        displayTimeSync(false);
        return;
    }
    else
    {
        // Get the current time from NTP (hours, minutes, seconds)
        int hour = timeinfo.tm_hour;
        int minute = timeinfo.tm_min;
        int second = timeinfo.tm_sec;
        int day = timeinfo.tm_mday;
        int month = timeinfo.tm_mon + 1;
        int year = timeinfo.tm_year - 100;

        Serial.printf("%d/%02d/%02d %02d:%02d:%02d\r\n", year, month, day, hour, minute, second);

        setDateTime(year, month, day, hour, minute, second);
        Serial.println("**Time Sync**");
        displayTimeSync(true);
    }
}