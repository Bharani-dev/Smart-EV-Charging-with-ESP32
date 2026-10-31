#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ---------------------------------------------------------------------
// Credentials (loaded from secrets.h which is ignored by Git)
// ---------------------------------------------------------------------
#if __has_include("secrets.h")
  #include "secrets.h"
#else
  #include "secrets.example.h"
#endif

// ---------------------------------------------------------------------
// Multi-Bay Station Configuration
// ---------------------------------------------------------------------
#define NUM_BAYS 3

struct BayConfig {
    const char* id;
    const char* token;
};

// Credentials for each bay on ThingsBoard
constexpr BayConfig BAYS[NUM_BAYS] = {
    {"BAY1", SECRET_BAY1_TOKEN},
    {"BAY2", SECRET_BAY2_TOKEN},
    {"BAY3", SECRET_BAY3_TOKEN}
};

// ---------------------------------------------------------------------
// Pin configuration — Bay 1 Physical Hardware (SRS Section 8.1)
// ---------------------------------------------------------------------
#define VOLTAGE_PIN   34
#define CURRENT_PIN   35
#define DHT_PIN       15
#define RELAY_PIN     26
#define BTN_PLUGIN    32
#define BTN_PLUGOUT   33
#define LED_GREEN     18
#define LED_YELLOW    19
#define LED_RED       21

#define DHT_TYPE DHT22

// Duty-cycle window for relay throttling (in milliseconds)
#define DUTY_CYCLE_WINDOW_MS 1000

// ---------------- WiFi Details ----------------
constexpr const char* WIFI_SSID = SECRET_WIFI_SSID;
constexpr const char* WIFI_PASS = SECRET_WIFI_PASS;
constexpr const char* WIFI_PASSWORD = WIFI_PASS;

// ---------------- ThingsBoard MQTT Broker ----------------
constexpr const char* MQTT_SERVER = "mqtt.thingsboard.cloud";
constexpr int MQTT_PORT = 1883;

#endif
