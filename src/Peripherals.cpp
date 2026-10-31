#include <Arduino.h>
#include <DHT.h>
#include "State.h"
#include "Peripherals.h"
#include "config.h"

DHT dht(DHT_PIN, DHT_TYPE);

void initPeripherals(void) {
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);

    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_RED, OUTPUT);

    pinMode(BTN_PLUGIN, INPUT_PULLUP);
    pinMode(BTN_PLUGOUT, INPUT_PULLUP);

    dht.begin();
}

void printval(void)
{
    // -------------------------------------------------------------
    // Bay 1: Read physical sensors (Potentiometers & DHT)
    // -------------------------------------------------------------
    int rawCurrent = analogRead(CURRENT_PIN);
    int rawVoltage = analogRead(VOLTAGE_PIN);

    // Map 12-bit ADC (0-4095) to electrical units
    bays[0].current = (rawCurrent / 4095.0f) * 40.0f; // 0 to 40 A
    bays[0].voltage = (rawVoltage / 4095.0f) * 250.0f; // 0 to 250 V
    bays[0].power = (bays[0].bayStatus == "CHARGING") ? (bays[0].voltage * bays[0].current * (bays[0].throttleLevel / 100.0f)) : 0.0f;

    // Read ambient temperature & humidity
    temperature = dht.readTemperature(false);
    humidity = dht.readHumidity();

    // -------------------------------------------------------------
    // Bay 2 & 3: Update simulated EV charging sessions
    // -------------------------------------------------------------
    if (bays[1].bayStatus == "CHARGING") {
        bays[1].power = bays[1].voltage * bays[1].current * (bays[1].throttleLevel / 100.0f);
        bays[1].energyWh += bays[1].power * (2.0f / 3600.0f); // Accumulate energy every 2 sec
    } else {
        bays[1].power = 0.0f;
    }

    if (bays[2].bayStatus == "CHARGING") {
        bays[2].power = bays[2].voltage * bays[2].current * (bays[2].throttleLevel / 100.0f);
        bays[2].energyWh += bays[2].power * (2.0f / 3600.0f);
    } else {
        bays[2].power = 0.0f;
    }

    if (bays[0].bayStatus == "CHARGING") {
        bays[0].energyWh += bays[0].power * (2.0f / 3600.0f);
    }

    // -------------------------------------------------------------
    // Serial Diagnostic Output
    // -------------------------------------------------------------
    Serial.println("\n========== MULTI-BAY STATUS ==========");
    for (int i = 0; i < NUM_BAYS; i++) {
        Serial.printf("[%s] %s | V: %.1fV | I: %.1fA | P: %.0fW | Thr: %d%% | Dec: %s\n",
                      bays[i].bayId.c_str(),
                      bays[i].bayStatus.c_str(),
                      bays[i].voltage,
                      bays[i].current,
                      bays[i].power,
                      bays[i].throttleLevel,
                      bays[i].loadDecision.c_str());
    }
    Serial.printf(">> Station Total Power: %.0f W / Limit: %.0f W\n", totalStationPower, maxStationLoadW);
    if (!isnan(temperature) && !isnan(humidity)) {
        Serial.printf(">> Station Environment: %.1f C | %.1f %%\n", temperature, humidity);
    }
    Serial.println("======================================");
}

void plug_status(void)
{
    // BTN_PLUGIN (pin 32): Green button - plugged in
    if (digitalRead(BTN_PLUGIN) == LOW) {
        if (bays[0].bayStatus != "CHARGING") {
            bays[0].bayStatus = "CHARGING";
            bays[0].sessionStartMs = millis();
            Serial.println(">> [BAY1] Plugged In: status -> CHARGING");
        }
    }

    // BTN_PLUGOUT (pin 33): Red button - plugged out
    if (digitalRead(BTN_PLUGOUT) == LOW) {
        if (bays[0].bayStatus != "FREE") {
            bays[0].bayStatus = "FREE";
            bays[0].sessionStartMs = 0;
            Serial.println(">> [BAY1] Plugged Out: status -> FREE");
        }
    }
}
