#ifndef _LIGHTNING_SENSOR_H_
#define _LIGHTNING_SENSOR_H_

#include "SensorSettings.h"
#include "SensorEvent.h"

class LightningSensor
{
  public:
    void detachInterruptPin(void);
    void attachInterruptPin(void);
    int begin(SensorSettings sensorSettings, bool enableInterruptPin = true);
    SparkFun_AS3935& getSensor();
    bool getSensorEvent(SensorEvent *sensorEvent);
};
#endif
