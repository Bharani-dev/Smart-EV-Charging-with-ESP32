#include "State.h"

// ---------------------------------------------------------------------
// Multi-bay initial states
// ---------------------------------------------------------------------
BayState bays[NUM_BAYS] = {
    // Bay 1: Controlled by physical hardware in Wokwi (pots, buttons, relay, LEDs)
    {
        .bayId = "BAY1",
        .bayStatus = "FREE",
        .voltage = 0.0f,
        .current = 0.0f,
        .power = 0.0f,
        .energyWh = 0.0f,
        .overloadActive = false,
        .manualOverrideActive = false,
        .throttleLevel = 100,
        .loadDecision = "ALLOW",
        .sessionStartMs = 0,
        .predictedArrivalProb = 0.0f,
        .predictedDurationMin = 0
    },
    // Bay 2: Software simulated EV session (active charging, longer duration)
    {
        .bayId = "BAY2",
        .bayStatus = "CHARGING",
        .voltage = 230.0f,
        .current = 16.0f,
        .power = 3680.0f,
        .energyWh = 1250.0f,
        .overloadActive = false,
        .manualOverrideActive = false,
        .throttleLevel = 100,
        .loadDecision = "ALLOW",
        .sessionStartMs = 0,
        .predictedArrivalProb = 0.65f,
        .predictedDurationMin = 35 // > 10 min, candidate for throttling under peak station load
    },
    // Bay 3: Software simulated EV session (active charging, nearly finished)
    {
        .bayId = "BAY3",
        .bayStatus = "CHARGING",
        .voltage = 230.0f,
        .current = 20.0f,
        .power = 4600.0f,
        .energyWh = 8400.0f,
        .overloadActive = false,
        .manualOverrideActive = false,
        .throttleLevel = 100,
        .loadDecision = "ALLOW",
        .sessionStartMs = 0,
        .predictedArrivalProb = 0.70f,
        .predictedDurationMin = 8 // <= 10 min, prioritized high-priority session
    }
};

// Station-wide state
float totalStationPower = 0.0f;
float temperature = 0.0f;
float humidity = 0.0f;
int lastHourOfDay = 12;

// Shared attributes (defaults until updated from ThingsBoard)
float maxStationLoadW = 7000.0f;   // 7 kW station power cap
float overloadCurrentA = 32.0f;    // 32 A per-bay limit
int peakTariffStartHr = 18;        // 6:00 PM
int peakTariffEndHr = 22;          // 10:00 PM
float predictionThreshold = 0.5f;
