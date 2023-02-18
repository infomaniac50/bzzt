#ifndef _SENSOR_EVENT_H_
#define _SENSOR_EVENT_H_

#include <cstdint>

inline const char *statusToString(std::uint8_t type)
{
  switch (type)
  {
    case INTERRUPT_STATUS::LIGHTNING:
      return "Lightning";
    case INTERRUPT_STATUS::DISTURBER_DETECT:
      return "Disturber";
    case INTERRUPT_STATUS::NOISE_TO_HIGH:
      return "Noise Floor Too High";
    default:
      return "No Event";
  }
}

struct SensorEvent
{
  std::uint8_t type;
  std::uint8_t distance;
  long energy;
};

#endif
