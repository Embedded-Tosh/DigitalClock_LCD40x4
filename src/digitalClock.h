#ifndef __DIGITALCLOCK_h__
#define __DIGITALCLOCK_h__

void SyncRTCwithServer(void);
void getDateTime(void);
void setDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec);
void ds1340_RTC_Init(void);
#endif
