#ifndef MAIN_H
#define MAIN_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <stdint.h>
#include <string>
#include "WebServer.h"

// Json document size to hold the commands send between client/server
#define COMMAND_DOC_SIZE 255
// Json document size to hold the config (depends on config size)
#define CONFIG_DOC_SIZE 20000
/*-----------------------------------------------------------------------------
 * Global parameters
 *
 * These parameters are used for dynamically changing runtime operation. They
 * can (optionally) be loaded from and writen to persistant storage.
 *
 * Animation init or draw routines need to apply these parameters to
 * dynamically set runtime parameters. Init only gets called when an animation
 * starts or restarts draw gets called every animation frame so choose wisely
 * where to apply. And let these parameters take effect. (Be careful of Timers)
 *
 * After creation of the config object, call load() to load the configuration
 * from the "config.json" file and apply the values to the config struct.
 *
 * If no "config.json" exists the config structs keeps the values supplied in
 * the code. After saveing a "config.json" is freshly created.
 *---------------------------------------------------------------------------*/
struct Config
{
  struct
  {
    uint16_t display_contrast = 255;
    float brightness = 1;
    int16_t display_brightness = 255;
  } power;

  struct
  {
    char ssid[32] = "TP-Link_58E0";
    char password[64] = "84248299";
    char hostname[64] = "dclock";
    uint16_t port = 80;
  } network;

  File open(const char *name, const char *mode)
  {
    if (!LittleFS.begin())
    {
      Serial.println("Error mounting LittleFS");
      return (File)0;
    }
    File file = LittleFS.open(name, mode);
    if (!file)
    {
      Serial.printf("Error opening file for %s\n", mode);
      return (File)0;
    }
    return file;
  }

  void load()
  {
    if (File file = open("/config.json", FILE_READ))
    {
      DynamicJsonDocument doc(CONFIG_DOC_SIZE);
      DeserializationError err = deserializeJson(doc, file);
      if (err)
        Serial.printf("Deserialization error while loading file: %s\n", err.c_str());
      else
        deserialize(doc);
      file.close();
    }
  }

  void save()
  {
    if (File file = open("/config.json", FILE_WRITE))
    {
      DynamicJsonDocument doc(CONFIG_DOC_SIZE);
      JsonObject root = doc.to<JsonObject>();
      serialize(root);
      // Save active animation to fs, but don't create a slider on the gui.
      JsonObject obj = root["settings"]["display"];
      size_t size = serializeJson(root, file);
      Serial.printf("%u bytes written to config.json\n", size);
      file.close();
    }
  }

  void reset()
  {
    if (LittleFS.remove("/config.json"))
    {
      Serial.printf("Deleted config.json\n");
      delay(1000);
    }
    ESP.restart();
  }

  void slider(JsonObject &node, const char *id, const char *name, float value,
              float min = 0, float max = 255, float step = 1)
  {
    JsonObject leaf = node.createNestedObject(id);
    leaf["name"] = name;
    leaf["type"] = "slider";
    leaf["value"] = value;
    leaf["min"] = min;
    leaf["max"] = max;
    leaf["step"] = step;
  }

  void checkbox(JsonObject &node, const char *id, const char *name,
                boolean value)
  {
    JsonObject leaf = node.createNestedObject(id);
    leaf["name"] = name;
    leaf["type"] = "checkbox";
    leaf["value"] = value;
  }

  void text(JsonObject &node, const char *id, const char *name,
            const char *value, uint8_t size)
  {
    JsonObject leaf = node.createNestedObject(id);
    leaf["name"] = name;
    leaf["type"] = "text";
    leaf["value"] = value;
    leaf["size"] = size;
  }

  void number(JsonObject &node, const char *id, const char *name, float value,
              float min = 0, float max = 255, float step = 1)
  {
    JsonObject leaf = node.createNestedObject(id);
    leaf["name"] = name;
    leaf["type"] = "number";
    leaf["value"] = value;
    leaf["min"] = min;
    leaf["max"] = max;
    leaf["step"] = step;
  }

  void serialize(JsonObject &root)
  {
    JsonObject settings = root.createNestedObject("settings");
    settings["name"] = "Configuration Settings";
    JsonObject obj;
    { // SETTINGS.DISPLAY
      obj = settings.createNestedObject("display");
      obj["name"] = "Display Settings";
      slider(obj, "display_contrast", "Display Contrast", power.display_contrast, 0, 255, 1);
      slider(obj, "display_brightness", "Display Brightness", power.display_brightness, 0, 255, 1);
    }
    { // SETTINGS.NETWORK
      obj = settings.createNestedObject("network");
      obj["name"] = "Network Settings";
      text(obj, "ssid", "Network SSID", network.ssid, 32);
      text(obj, "password", "Password", network.password, 64);
      text(obj, "hostname", "Hostname", network.hostname, 64);
      number(obj, "port", "Port", network.port, 0x0000, 0xffff, 1);
    }
  };

  void deserialize(JsonDocument &doc)
  {
    { // SETTINGS.DISPLAY
      JsonObject obj = doc["settings"]["display"];
      power.display_contrast = obj["display_contrast"]["value"] | power.display_contrast;
      power.display_brightness = obj["display_brightness"]["value"] | power.display_brightness;
    }
    { // SETTINGS.NETWORK
      JsonObject obj = doc["settings"]["network"];
      strlcpy(network.ssid, obj["ssid"]["value"] | network.ssid, sizeof(network.ssid));
      strlcpy(network.password, obj["password"]["value"] | network.password, sizeof(network.password));
      strlcpy(network.hostname, obj["hostname"]["value"] | network.hostname, sizeof(network.hostname));
      network.port = obj["port"]["value"] | network.port;
    }
  }

  void execute(uint8_t *payload)
  {
    DynamicJsonDocument cmd(COMMAND_DOC_SIZE);
    DeserializationError err = deserializeJson(cmd, payload);
    if (err)
    {
      Serial.printf("Deserialization error (execute): %s\n", err.c_str());
      return;
    }

    String event = cmd["event"];
    // Serial.println(event);
    if (event.equals("activate"))
    {
      // Synchronize all clients to turn off cycle animations
      cmd.clear();
      cmd["event"] = "update";
      JsonObject settings = cmd.createNestedObject("settings");
      JsonObject display = settings.createNestedObject("display");
      JsonObject object = display.createNestedObject("play_one");
      uint8_t buffer[COMMAND_DOC_SIZE];
      serializeJson(cmd, buffer);
      WebServer::broadcast(buffer);
    }
    else if (event.equals("update"))
    {
      cmd.remove("event");
      deserialize(cmd);
      if (cmd["settings"]["display"]["display_brightness"]["value"] == 0)
        ledcWrite(1, power.display_brightness);
      if (cmd["settings"]["display"]["display_brightness"]["value"])
      {
        // Serial.println(power.display_brightness);
        ledcWrite(1, power.display_brightness);
      }
      if (cmd["settings"]["display"]["display_contrast"]["value"])
      {
        // Serial.println(power.display_contrast);
        ledcWrite(0, power.display_contrast);
      }
    }
    else if (event.equals("save"))
    {
      save();
    }
    else if (event.equals("reset"))
    {
      reset();
    }
  }
};
// All cpp files that include this link to a single config struct
extern struct Config config;
#endif