#ifndef BAY_NETWORK_H
#define BAY_NETWORK_H

#include <WiFiClient.h>
#include <PubSubClient.h>
#include "config.h"

extern WiFiClient espClients[NUM_BAYS];
extern PubSubClient mqttClients[NUM_BAYS];

void connectWiFi();
void connectMQTT();
void maintainNetwork();

#endif
