#include <ArduinoJson.h>
#include "rpc.h"
#include "bay_network.h"
#include "State.h"
#include "config.h"

// ---------------------------------------------------------------------
// SRS 8.5: Multi-Bay RPC command handling
// ---------------------------------------------------------------------
void handleRpc(int bayIndex, String requestId, char* payload) {
  if (bayIndex < 0 || bayIndex >= NUM_BAYS) return;

  StaticJsonDocument<200> doc;
  if (deserializeJson(doc, payload)) return;

  String method = doc["method"] | "";
  JsonObject params = doc["params"];

  StaticJsonDocument<200> response;

  if (method == "setRelayState") {
    bool state = params["state"] | false;
    bays[bayIndex].manualOverrideActive = true;
    bays[bayIndex].throttleLevel = state ? 100 : 0;
    bays[bayIndex].loadDecision = state ? "ALLOW" : "MANUAL_OFF";
    response["success"] = true;
    response["throttleLevel"] = bays[bayIndex].throttleLevel;
    Serial.printf(">> RPC [%s] setRelayState(%d) - manual override engaged.\n", bays[bayIndex].bayId.c_str(), state);

  } else if (method == "setThrottle") {
    int level = params["level"] | 100;
    level = constrain(level, 0, 100);
    bays[bayIndex].manualOverrideActive = true;
    bays[bayIndex].throttleLevel = level;
    bays[bayIndex].loadDecision = "MANUAL_THROTTLE";
    response["success"] = true;
    response["throttleLevel"] = bays[bayIndex].throttleLevel;
    Serial.printf(">> RPC [%s] setThrottle(%d) - manual override engaged.\n", bays[bayIndex].bayId.c_str(), level);

  } else {
    response["success"] = false;
    response["error"] = "unknown method";
  }

  char buffer[220];
  serializeJson(response, buffer);
  mqttClients[bayIndex].publish(("v1/devices/me/rpc/response/" + requestId).c_str(), buffer);
}
