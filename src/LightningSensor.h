#ifndef _LIGHTNING_SENSOR_H_
#define _LIGHTNING_SENSOR_H_

#include "SensorSettings.h"
#include "SensorEvent.h"

class LightningSensor
{
  private:
    void attachInterruptPin(void);
    void detachInterruptPin(void);

  public:
    void begin(SensorSettings sensorSettings);
    bool getSensorEvent(SensorEvent *sensorEvent);
};
#endif
