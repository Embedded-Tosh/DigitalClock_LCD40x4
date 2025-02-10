#include <LiquidCrystal.h>
#include "Arduino.h"
#include "LCD_40X4.h"

// Create An LCD Object. Signals: [ RS, EN, D4, D5, D6, D7 ]
LiquidCrystal lcd(33, 25, 26, 27, 14, 13);
LiquidCrystal lcd1(33, 32, 26, 27, 14, 13);

// Declare previous time values for comparison
int prevHourTens = -1, prevHourOnes = -1;
int prevMinuteTens = -1, prevMinuteOnes = -1;
int prevSecondTens = -1, prevSecondOnes = -1;

// Custom characters
byte customChar1[] = {0x07, 0x07, 0x0F, 0x1F, 0x00, 0x00, 0x00, 0x00};
byte customChar2[] = {0x18, 0x1C, 0x1E, 0x1F, 0x00, 0x00, 0x00, 0x00};
byte customChar3[] = {0x1F, 0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00};
byte customChar4[] = {0x1F, 0x1E, 0x1C, 0x18, 0x00, 0x00, 0x00, 0x00};
byte customChar5[] = {0x1F, 0x0F, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00};
byte customChar6[] = {0x03, 0x07, 0x0F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};
byte customChar7[] = {0x18, 0x1C, 0x1E, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};
byte WifiConnect[] = {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x00, 0x00};
byte WifiDisconnect[] = {0x00, 0x00, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x00};

// Digit map (grid sequences)
int digitMap[10][3][3] =
    {
        {{5, 2, 6}, {0XFF, ' ', 0XFF}, {4, 2, 3}},        // 0
        {{0, 0XFF, ' '}, {' ', 0XFF, ' '}, {4, 2, 3}},    // 1
        {{0, 2, 6}, {5, 2, 3}, {4, 2, 3}},                // 2
        {{0, 2, 6}, {' ', 2, 0XFF}, {4, 2, 3}},           // 3
        {{0XFF, ' ', 0XFF}, {4, 2, 0XFF}, {' ', ' ', 2}}, // 4
        {{0XFF, 2, 3}, {4, 2, 6}, {4, 2, 3}},             // 5
        {{5, 2, 3}, {0XFF, 2, 6}, {4, 2, 3}},             // 6
        {{4, 2, 0XFF}, {' ', 5, 3}, {' ', 2, ' '}},       // ' '
        {{5, 2, 6}, {0XFF, 2, 0XFF}, {4, 2, 3}},          // 8
        {{5, 2, 6}, {4, 2, 0XFF}, {4, 2, 3}}              // 9
};

// Display Custom Char Display @ 3X3
void displayDigit(int col, int digit)
{
    for (int row = 0; row < 3; row++)
    {
        if (row == 2)
        {
            lcd1.setCursor(col, 0);
            for (int col = 0; col < 3; col++)
            {
                int charIndex = digitMap[digit][row][col];
                lcd1.write(charIndex);
            }
        }
        else
        {
            lcd.setCursor(col, row);
            for (int col = 0; col < 3; col++)
            {
                int charIndex = digitMap[digit][row][col];
                lcd.write(charIndex);
            }
        }
    }
}

// Function to load custom characters
void loadCustomChars(void)
{
    lcd.createChar(0, customChar1);
    lcd.createChar(1, customChar2);
    lcd.createChar(2, customChar3);
    lcd.createChar(3, customChar4);
    lcd.createChar(4, customChar5);
    lcd.createChar(5, customChar6);
    lcd.createChar(6, customChar7);

    lcd1.createChar(0, customChar1);
    lcd1.createChar(1, customChar2);
    lcd1.createChar(2, customChar3);
    lcd1.createChar(3, customChar4);
    lcd1.createChar(4, customChar5);
    lcd1.createChar(5, customChar6);
    lcd1.createChar(6, customChar7);
}

void displayWifiConnected(void)
{
    lcd.createChar(7, WifiConnect);
    lcd.setCursor(39, 0);
    lcd.write(7);
}

void displayWifiDisconnected(void)
{
    lcd.createChar(7, WifiDisconnect);
    lcd.setCursor(39, 0);
    lcd.write(7);
}

unsigned int lastProgress = 0; // Tracks the previous progress value
unsigned int currProgress = 0;

void setProgress(void)
{
    lastProgress = 0; // Tracks the previous progress value
    currProgress = 0;
    lcd1.setCursor(0, 1);
    lcd1.print("OTA Update Progress: ");
}

void setProgressfail(void)
{
    lcd1.setCursor(0, 1);
    lcd1.print("OTA Update Progress: Fail");
}

void displayOtaProcess(unsigned int progress, unsigned int total)
{
    currProgress = (progress / (total / 100.0));
    if (currProgress > lastProgress)
    {
        lastProgress = currProgress; // Update the last progress value
        lcd1.setCursor(21, 1);
        lcd1.print((progress / (total / 100.0)));
    }
}

void displayTimeSync(bool state)
{
    lcd1.setCursor(0, 1);
    if (state)
        lcd1.print("*** Time Sync ***");
    delay(2000);
    lcd1.setCursor(0, 1);
    lcd1.print("                 ");
}

// Function to display time in HH:MM:SS format using custom digits
void displayTime(int col, int hour, int minute, int second, int day, int month, int year, char *monthName, char *weekDay)
{
    bool isPM = hour >= 12;
    int hour12 = hour % 12;
    if (hour12 == 0)
    {
        hour12 = 12; // Adjust for midnight or noon
    }

    lcd.setCursor(0, 1);
    lcd.printf("%.3s %2d", weekDay, day);
    lcd1.setCursor(0, 0);
    lcd1.printf("%.3s %4d", monthName, year);

    // Check if the hour tens place has changed
    if (hour12 / 10 != prevHourTens)
    {
        displayDigit(0 + col, hour12 / 10); // Display new tens of hour
        prevHourTens = hour12 / 10;         // Update previous tens of hour value
    }

    // Check if the hour ones place has changed
    if (hour12 % 10 != prevHourOnes)
    {
        displayDigit(3 + col, hour12 % 10); // Display new ones of hour
        prevHourOnes = hour12 % 10;         // Update previous ones of hour value
    }

    // Check if the minute tens place has changed
    if (minute / 10 != prevMinuteTens)
    {
        displayDigit(7 + col, minute / 10); // Display new tens of minute
        prevMinuteTens = minute / 10;       // Update previous tens of minute value
    }

    // Check if the minute ones place has changed
    if (minute % 10 != prevMinuteOnes)
    {
        displayDigit(10 + col, minute % 10); // Display new ones of minute
        prevMinuteOnes = minute % 10;        // Update previous ones of minute value
    }

    // Check if the second tens place has changed
    if (second / 10 != prevSecondTens)
    {
        displayDigit(14 + col, second / 10); // Display new tens of second
        prevSecondTens = second / 10;        // Update previous tens of second value
    }

    // Check if the second ones place has changed
    if (second % 10 != prevSecondOnes)
    {
        displayDigit(17 + col, second % 10); // Display new ones of second
        prevSecondOnes = second % 10;        // Update previous ones of second value
    }

    lcd.setCursor(20 + col, 0);
    lcd.print(isPM ? "PM" : "AM");

    // Update the colons (":") only once
    lcd.setCursor(6 + col, 0);
    lcd.write('o');
    lcd.setCursor(13 + col, 0);
    lcd.write('o');

    lcd.setCursor(6 + col, 1);
    lcd.write('o');
    lcd.setCursor(13 + col, 1);
    lcd.write('o');
}

void displayTemphumi(float temp, float humi)
{
    // Validate temperature
    if (isnan(temp) || temp == 0.0)
        temp = 0.0; // Set to zero if invalid

    // Validate humidity
    if (isnan(humi) || humi == 0.0)
        humi = 0.0; // Set to zero if invalid

    lcd.setCursor(36, 1);
    lcd.print(temp, 0);
    lcd.setCursor(38, 1);
    lcd.write(0xDF);
    lcd.setCursor(39, 1);
    lcd.write('C');

    lcd1.setCursor(36, 0);
    lcd1.print(humi, 0);
    lcd.setCursor(38, 1);
    lcd1.write('%');
}

void lcd_40X4_Init(void)
{
    // Initialize The LCD. Parameters: [ Columns, Rows ]
    lcd.begin(40, 2);
    lcd1.begin(40, 2);

    loadCustomChars();
    // Clears The LCD Display
    lcd.clear();
    lcd1.clear();
}