#ifndef _SENSOR_SETTINGS_H_
#define _SENSOR_SETTINGS_H_

#include <cstdint>
#include <SparkFun_AS3935.h>

// Values for modifying the IC's settings. All of these values are set to their
// default values.
struct SensorSettings
{
    std::uint8_t noiseFloor = 2;
    std::uint8_t watchdogThreshold = 2;
    std::uint8_t spikeRejection = 2;
    std::uint8_t lightningThreshold = 1;
    std::uint8_t tuningCapacitor = 0;
    std::uint8_t sensorLocation = INDOOR;
    bool reportDisturber = false;
};

#endif
