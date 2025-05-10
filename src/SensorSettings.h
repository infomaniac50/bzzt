#ifndef _SENSOR_SETTINGS_H_
#define _SENSOR_SETTINGS_H_

#include <cstdint>
#include <SparkFun_AS3935.h>
#include <Preferences.h>

#define DEFAULT_SENSOR_SETTING_NOISE_FLOOR (uint8_t)2
#define DEFAULT_SENSOR_SETTING_WATCHDOG_THRESHOLD (uint8_t)2
#define DEFAULT_SENSOR_SETTING_SPIKE_REJECTION (uint8_t)2
#define DEFAULT_SENSOR_SETTING_LIGHTNING_THRESHOLD (uint8_t)1
#define DEFAULT_SENSOR_SETTING_TUNING_CAPACITOR (uint8_t)0
#define DEFAULT_SENSOR_SETTING_SENSOR_LOCATION (uint8_t)INDOOR
#define DEFAULT_SENSOR_SETTING_REPORT_DISTURBER (bool)false
#define DEFAULT_SENSOR_SETTING_APP_NAME "bzzt_sensor"

// Values for modifying the IC's settings. All of these values are set to their
// default values.
struct SensorSettings
{
  std::uint8_t noiseFloor = DEFAULT_SENSOR_SETTING_NOISE_FLOOR;
  std::uint8_t watchdogThreshold = DEFAULT_SENSOR_SETTING_WATCHDOG_THRESHOLD;
  std::uint8_t spikeRejection = DEFAULT_SENSOR_SETTING_SPIKE_REJECTION;
  std::uint8_t lightningThreshold = DEFAULT_SENSOR_SETTING_LIGHTNING_THRESHOLD;
  std::uint8_t tuningCapacitor = DEFAULT_SENSOR_SETTING_TUNING_CAPACITOR;
  std::uint8_t sensorLocation = DEFAULT_SENSOR_SETTING_SENSOR_LOCATION;
  bool reportDisturber = DEFAULT_SENSOR_SETTING_REPORT_DISTURBER;
};

class SensorPreferences {
  private:
    Preferences preferences;

  public:
    void save(SensorSettings* settings);
    void load(SensorSettings* settings);
    bool clear();
};
#endif
