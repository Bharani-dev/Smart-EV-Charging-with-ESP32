#include <Arduino.h>
#include <WiFi.h>
#include "State.h"
#include "config.h"
#include "Peripherals.h"
#include "bay_network.h"
#include "optimization.h"
#include "Edge_AI.h"
#include "telemetry.h"
#include "rpc.h"
#include "attributes.h"

unsigned long now = 0;
unsigned long last_telemetry = 0;
unsigned long last_ai = 0;

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting ESP32 Multi-Bay EV Charging Station (Bays 1-3)...");

    initPeripherals();
    connectWiFi();
    connectMQTT();
}

void loop()
{
    now = millis();

    // Maintain network connections for all 3 bays
    maintainNetwork();

    // Process Bay 1 button events (plug-in / plug-out)
    plug_status();

    // Apply optimization policy across all bays, duty cycle Bay 1 relay, update LEDs
    runOptimization();
    applyRelayDutyCycle();
    updateLeds();

    // Read sensors and publish telemetry for all 3 bays every 2 seconds
    if (now - last_telemetry >= 2000) {
        last_telemetry = now;
        printval();
        publishTelemetry();
    }

    // Run Edge AI model inference for all bays every 5 seconds
    if (now - last_ai >= 5000) {
        last_ai = now;
        runEdgeAIInference();
    }
}
