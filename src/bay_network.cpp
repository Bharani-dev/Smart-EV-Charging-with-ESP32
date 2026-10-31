#include <WiFi.h>
#include <ArduinoJson.h>
#include "bay_network.h"
#include "config.h"
#include "attributes.h"
#include "rpc.h"

WiFiClient espClients[NUM_BAYS];
PubSubClient mqttClients[NUM_BAYS] = {
    PubSubClient(espClients[0]),
    PubSubClient(espClients[1]),
    PubSubClient(espClients[2])
};

void handleBayMqttMessage(int bayIndex, char* topic, byte* payload, unsigned int length);

void mqttCallback0(char* topic, byte* payload, unsigned int length) {
    handleBayMqttMessage(0, topic, payload, length);
}

void mqttCallback1(char* topic, byte* payload, unsigned int length) {
    handleBayMqttMessage(1, topic, payload, length);
}

void mqttCallback2(char* topic, byte* payload, unsigned int length) {
    handleBayMqttMessage(2, topic, payload, length);
}

typedef void (*MqttCbFunc)(char*, byte*, unsigned int);
const MqttCbFunc callbacks[NUM_BAYS] = {mqttCallback0, mqttCallback1, mqttCallback2};

void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(300);
    Serial.print(".");
  }
  Serial.println(WiFi.status() == WL_CONNECTED ? " connected." : " FAILED (will retry).");
}

void connectBayMQTT(int i) {
  if (WiFi.status() != WL_CONNECTED) return;
  mqttClients[i].setServer(MQTT_SERVER, MQTT_PORT);
  mqttClients[i].setCallback(callbacks[i]);

  Serial.printf("Connecting %s to ThingsBoard MQTT...", BAYS[i].id);
  if (mqttClients[i].connect(BAYS[i].id, BAYS[i].token, NULL)) {
    Serial.printf(" %s connected.\n", BAYS[i].id);
    mqttClients[i].subscribe("v1/devices/me/attributes");
    mqttClients[i].subscribe("v1/devices/me/attributes/response/+");
    mqttClients[i].subscribe("v1/devices/me/rpc/request/+");
    if (i == 0) {
      requestSharedAttributes();
    }
  } else {
    Serial.printf(" %s failed, rc=%d\n", BAYS[i].id, mqttClients[i].state());
  }
}

void connectMQTT() {
  for (int i = 0; i < NUM_BAYS; i++) {
    connectBayMQTT(i);
  }
}

void maintainNetwork() {
  if (WiFi.status() != WL_CONNECTED) return;
  for (int i = 0; i < NUM_BAYS; i++) {
    if (!mqttClients[i].connected()) {
      connectBayMQTT(i);
    }
    mqttClients[i].loop();
  }
}

void handleBayMqttMessage(int bayIndex, char* topic, byte* payload, unsigned int length) {
  String topicStr = String(topic);
  char buf[400];
  unsigned int n = length < sizeof(buf) - 1 ? length : sizeof(buf) - 1;
  memcpy(buf, payload, n);
  buf[n] = '\0';

  Serial.printf("[MQTT << %s] %s %s\n", BAYS[bayIndex].id, topicStr.c_str(), buf);

  if (topicStr.startsWith("v1/devices/me/rpc/request/")) {
    String requestId = topicStr.substring(topicStr.lastIndexOf('/') + 1);
    handleRpc(bayIndex, requestId, buf);
    return;
  }

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, buf);
  if (err) return;

  JsonObject attrs = doc.containsKey("shared") ? doc["shared"].as<JsonObject>() : doc.as<JsonObject>();
  applySharedAttributes(attrs);
}
