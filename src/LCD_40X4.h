#ifndef __LCD_40X4_h__
#define __LCD_40X4_h__

void displayWifiConnected(void);
void displayWifiDisconnected(void);
void setProgressfail(void);
void setProgress(void);
void displayOtaProcess(unsigned int progress, unsigned int total);
void displayTimeSync(bool state);
void displayTemphumi(float temp, float humi);
void displayTime(int col, int hour, int minute, int second, int day, int month, int year, char *monthName, char *weekDay);
void lcd_40X4_Init(void);

#endif