#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <Arduino.h>
#include <stdint.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include "esp_wifi.h"
#include "time.h"
#include "LCD_40X4.h"
#include <DHT22.h>
#include "digitalClock.h"
#include "WebServer.h"
#include "Config.h"

#define LED_PIN 2
/*------------------------------------------------------------------------------
 * Globals
 *----------------------------------------------------------------------------*/
// Global configuration parameters
Config config;
// TaskHandle for running task on different core
TaskHandle_t WEB_Task;
void web_task(void *);

// setting PWM properties

#define LED_GPIO 2
#define PWM1_Ch 0
#define PWM1_Res 8
#define PWM1_Freq 5000

#define CONTRASS_PIN 15
#define PWM2_Ch 1
#define PWM2_Res 8
#define PWM2_Freq 5000

#define pinDATA 4 // SDA, or almost any other I/O pin

DHT22 dht22(pinDATA);

void setup()
{
  Serial.begin(115200);
  // Load config from file system
  config.load();

  lcd_40X4_Init();
  // Start WiFi
  // pinMode(15, OUTPUT);
  ledcAttachPin(CONTRASS_PIN, PWM1_Ch);
  ledcSetup(PWM1_Ch, PWM1_Freq, PWM1_Res);

  ledcAttachPin(LED_GPIO, PWM2_Ch);
  ledcSetup(PWM2_Ch, PWM2_Freq, PWM2_Res);

  ds1340_RTC_Init();

  ledcWrite(PWM1_Ch, 190);
  ledcWrite(PWM2_Ch, 1);
  //  analogWrite(2, 255 / 2);

  // Create task on core 0
  xTaskCreatePinnedToCore(web_task, "WEB", 20000, NULL, 8, &WEB_Task, 0);
}

void loop()
{

  float t = dht22.getTemperature();
  float h = dht22.getHumidity();

  displayTemphumi(t, h);
  getDateTime();

  delay(1000);
}
/*------------------------------------------------------------------------------
 * Webserver
 *----------------------------------------------------------------------------*/
void web_task(void *parameter)
{
  Serial.printf("Webserver running on core %d\n", xPortGetCoreID());
  // Initialize web server communication
  WebServer::begin();
  static float LED_PHASE = 0;
  while (true)
  {
    // Prevents watchdog timeout
    vTaskDelay(1);
    // Check for Web server events
    WebServer::update();
    // // Blink led button
    // LED_PHASE += 0.001f;
    // analogWrite(LED_PIN, sinf(LED_PHASE) * 255);
  }
}