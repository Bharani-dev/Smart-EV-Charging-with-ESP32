#ifndef STATE_H
#define STATE_H

#include <Arduino.h>
#include "config.h"

// ---------------------------------------------------------------------
// State for an individual Charging Bay
// ---------------------------------------------------------------------
struct BayState {
    String bayId;
    String bayStatus;          // "FREE" or "CHARGING"
    float voltage;             // Volts
    float current;             // Amps
    float power;               // Watts
    float energyWh;            // Accumulated Watt-hours
    bool overloadActive;
    bool manualOverrideActive;
    int throttleLevel;         // 0 to 100 (%)
    String loadDecision;       // "ALLOW", "THROTTLE", "STANDBY_EXPECTING", "LOW_DEMAND", etc.
    unsigned long sessionStartMs;
    float predictedArrivalProb;
    int predictedDurationMin;
};

// ---------------------------------------------------------------------
// Live multi-bay states (bays[0] = BAY1, bays[1] = BAY2, bays[2] = BAY3)
// ---------------------------------------------------------------------
extern BayState bays[NUM_BAYS];

// Station-wide state
extern float totalStationPower;
extern float temperature;
extern float humidity;
extern int lastHourOfDay;

// Station-wide shared attributes (configurable via ThingsBoard)
extern float maxStationLoadW;
extern float overloadCurrentA;
extern int peakTariffStartHr;
extern int peakTariffEndHr;
extern float predictionThreshold;

#endif
