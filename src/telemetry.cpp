#include <ArduinoJson.h>
#include "telemetry.h"
#include "bay_network.h"
#include "State.h"
#include "config.h"

// ---------------------------------------------------------------------
// Multi-Bay FR-7 telemetry publish.
// Publishes live telemetry for all 3 bays to their ThingsBoard devices.
// ---------------------------------------------------------------------
void publishTelemetry() {
  for (int i = 0; i < NUM_BAYS; i++) {
    if (!mqttClients[i].connected()) continue;

    StaticJsonDocument<350> doc;
    doc["bayId"] = bays[i].bayId;
    doc["voltage"] = round(bays[i].voltage * 10) / 10.0;
    doc["current"] = round(bays[i].current * 10) / 10.0;
    doc["power"] = round(bays[i].power * 10) / 10.0;
    doc["energyWh"] = round(bays[i].energyWh * 10) / 10.0;
    doc["temperature"] = round(temperature * 10) / 10.0;
    doc["humidity"] = round(humidity * 10) / 10.0;
    doc["bayStatus"] = bays[i].bayStatus;
    doc["throttleLevel"] = bays[i].throttleLevel;
    doc["loadDecision"] = bays[i].loadDecision;
    doc["overloadActive"] = bays[i].overloadActive;
    doc["manualOverrideActive"] = bays[i].manualOverrideActive;
    doc["predictedDurationMin"] = bays[i].predictedDurationMin;
    doc["predictedArrivalProb"] = round(bays[i].predictedArrivalProb * 100) / 100.0;
    doc["totalStationPower"] = round(totalStationPower * 10) / 10.0;

    char buffer[350];
    serializeJson(doc, buffer);

    mqttClients[i].publish("v1/devices/me/telemetry", buffer);
    Serial.printf("[MQTT >> %s] %s\n", bays[i].bayId.c_str(), buffer);
  }
}
