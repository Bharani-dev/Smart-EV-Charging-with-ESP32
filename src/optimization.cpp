#include <Arduino.h>
#include "optimization.h"
#include "config.h"
#include "State.h"

// ---------------------------------------------------------------------
// SRS 8.7: Multi-Bay Station-Wide Load Optimization
// ---------------------------------------------------------------------
void runOptimization() {
  // 1. Calculate total station power across all bays
  totalStationPower = 0.0f;
  for (int i = 0; i < NUM_BAYS; i++) {
    totalStationPower += bays[i].power;
  }

  // 2. Evaluate optimization policies for each bay
  for (int i = 0; i < NUM_BAYS; i++) {
    // Skip bays currently under manual operator override (RPC)
    if (bays[i].manualOverrideActive) {
      continue;
    }

    bays[i].overloadActive = false;

    // Policy when bay is FREE (idle)
    if (bays[i].bayStatus != "CHARGING") {
      bool peakHour = (lastHourOfDay >= peakTariffStartHr && lastHourOfDay <= peakTariffEndHr);

      if (peakHour && bays[i].predictedArrivalProb >= predictionThreshold) {
        bays[i].loadDecision = "STANDBY_EXPECTING";
      } else if (peakHour) {
        bays[i].loadDecision = "LOW_DEMAND";
      } else {
        bays[i].loadDecision = "ALLOW";
      }
      continue;
    }

    // Policy when bay is CHARGING
    if (bays[i].bayStatus == "CHARGING") {
      // Priority 1: Per-bay overcurrent protection
      if (bays[i].current > overloadCurrentA) {
        bays[i].loadDecision = "THROTTLE";
        bays[i].throttleLevel = 50;
        bays[i].overloadActive = true;
        Serial.printf("!! [%s] Overcurrent (%.1f A > %.1f A) -> THROTTLE @ 50%%.\n",
                      bays[i].bayId.c_str(), bays[i].current, overloadCurrentA);
      }
      // Priority 2: Station-wide power cap exceeded
      else if (totalStationPower > maxStationLoadW) {
        // Prioritize sessions closer to completion (predictedDurationMin <= 10 keep 100%)
        if (bays[i].predictedDurationMin > 10) {
          bays[i].loadDecision = "THROTTLE";
          bays[i].throttleLevel = 70;
          Serial.printf("!! Station cap exceeded (%.0f W > %.0f W) -> THROTTLE [%s] @ 70%% (rem: %d min).\n",
                        totalStationPower, maxStationLoadW, bays[i].bayId.c_str(), bays[i].predictedDurationMin);
        } else {
          bays[i].loadDecision = "ALLOW";
          bays[i].throttleLevel = 100;
          Serial.printf("!! Station cap exceeded -> [%s] KEEP FULL (rem: %d min <= 10 min).\n",
                        bays[i].bayId.c_str(), bays[i].predictedDurationMin);
        }
      }
      // Normal operation: Full power allowed
      else {
        bays[i].loadDecision = "ALLOW";
        bays[i].throttleLevel = 100;
      }
    }
  }
}

// ---------------------------------------------------------------------
// FR-6: Apply throttleLevel to Bay 1 relay via duty-cycling
// ---------------------------------------------------------------------
void applyRelayDutyCycle() {
  if (bays[0].bayStatus != "CHARGING") {
    digitalWrite(RELAY_PIN, LOW);
    return;
  }
  if (bays[0].throttleLevel >= 100) {
    digitalWrite(RELAY_PIN, HIGH);
    return;
  }
  if (bays[0].throttleLevel <= 0) {
    digitalWrite(RELAY_PIN, LOW);
    return;
  }
  unsigned long phase = millis() % DUTY_CYCLE_WINDOW_MS;
  unsigned long onTime = (DUTY_CYCLE_WINDOW_MS * bays[0].throttleLevel) / 100;
  digitalWrite(RELAY_PIN, phase < onTime ? HIGH : LOW);
}

// ---------------------------------------------------------------------
// Update Bay 1 status LEDs
// ---------------------------------------------------------------------
void updateLeds() {
  digitalWrite(LED_RED, bays[0].overloadActive ? HIGH : LOW);
  digitalWrite(LED_GREEN, (bays[0].bayStatus == "FREE" && !bays[0].overloadActive) ? HIGH : LOW);
  digitalWrite(LED_YELLOW, (bays[0].bayStatus == "CHARGING" && !bays[0].overloadActive) ? HIGH : LOW);
}
