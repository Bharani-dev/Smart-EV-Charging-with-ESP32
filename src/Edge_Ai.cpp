#include <Arduino.h>
#include <time.h>
#include "Edge_AI.h"
#include "State.h"
#include "Peripherals.h"
#include "Model.h"   // predictArrival(), predictDuration() — see ai_training/

// Commute-hour pattern used to generate the training data
float historicalArrivalRate(int hour) {
  if (hour >= 8 && hour <= 10) return 0.62f;
  if (hour >= 18 && hour <= 21) return 0.58f;
  if (hour >= 0 && hour <= 5) return 0.06f;
  if (hour >= 11 && hour <= 17) return 0.30f;
  return 0.18f;
}

float recentAvgCurrent(int bayIndex) {
  if (bayIndex >= 0 && bayIndex < NUM_BAYS) {
    return bays[bayIndex].current;
  }
  return 0.0f;
}

void runEdgeAIInference() {
  struct tm timeinfo;
  int hourOfDay = 12, dayOfWeek = 0;
  if (getLocalTime(&timeinfo, 100)) {
    hourOfDay = timeinfo.tm_hour;
    dayOfWeek = timeinfo.tm_wday;
  }
  lastHourOfDay = hourOfDay;
  float histRate = historicalArrivalRate(hourOfDay);

  for (int i = 0; i < NUM_BAYS; i++) {
    float bayOccupiedF = (bays[i].bayStatus == "CHARGING") ? 1.0f : 0.0f;
    float sessionElapsedMin = (bays[i].bayStatus == "CHARGING" && bays[i].sessionStartMs > 0)
                                  ? (millis() - bays[i].sessionStartMs) / 60000.0f
                                  : 0.0f;

    bays[i].predictedArrivalProb = predictArrival(hourOfDay, dayOfWeek, bayOccupiedF,
                                                  recentAvgCurrent(i), sessionElapsedMin, histRate);

    if (bays[i].bayStatus == "CHARGING") {
      float pred = predictDuration(hourOfDay, dayOfWeek, bayOccupiedF,
                                   recentAvgCurrent(i), sessionElapsedMin, histRate);
      bays[i].predictedDurationMin = (int)pred;
    } else {
      bays[i].predictedDurationMin = 0;
    }
  }
}
