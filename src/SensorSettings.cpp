#include "SensorSettings.h"


void SensorPreferences::save(SensorSettings* settings) {
  preferences.begin(DEFAULT_SENSOR_SETTING_APP_NAME, false);
  preferences.putUChar("nf", settings->noiseFloor);
  preferences.putUChar("wt", settings->watchdogThreshold);
  preferences.putUChar("sr", settings->spikeRejection);
  preferences.putUChar("lt", settings->lightningThreshold);
  preferences.putUChar("tc", settings->tuningCapacitor);
  preferences.putUChar("sl", settings->sensorLocation);
  preferences.putBool("rd", settings->reportDisturber);
  preferences.end();
}

void SensorPreferences::load(SensorSettings* settings) {
  preferences.begin(DEFAULT_SENSOR_SETTING_APP_NAME, true);
  settings->noiseFloor = preferences.getUChar("nf", DEFAULT_SENSOR_SETTING_NOISE_FLOOR);
  settings->watchdogThreshold = preferences.getUChar("wt", DEFAULT_SENSOR_SETTING_WATCHDOG_THRESHOLD);
  settings->spikeRejection = preferences.getUChar("sr", DEFAULT_SENSOR_SETTING_SPIKE_REJECTION);
  settings->lightningThreshold = preferences.getUChar("lt", DEFAULT_SENSOR_SETTING_LIGHTNING_THRESHOLD);
  settings->tuningCapacitor = preferences.getUChar("tc", DEFAULT_SENSOR_SETTING_TUNING_CAPACITOR);
  settings->sensorLocation = preferences.getUChar("sl", DEFAULT_SENSOR_SETTING_SENSOR_LOCATION);
  settings->reportDisturber = preferences.getBool("rd", DEFAULT_SENSOR_SETTING_REPORT_DISTURBER);
  preferences.end();
}

bool SensorPreferences::clear() {
  preferences.begin(DEFAULT_SENSOR_SETTING_APP_NAME, false);
  bool status = preferences.clear();
  preferences.end();
  return status;
}