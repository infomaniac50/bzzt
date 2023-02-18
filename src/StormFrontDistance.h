#ifndef _STORM_FRONT_DISTANCE_H_
#define _STORM_FRONT_DISTANCE_H_

#include <cstdint>

enum StormFrontDistance : std::uint8_t
{
  OUT_OF_RANGE = 0b111111,
  DISTANCE_40KM = 0b101000,
  DISTANCE_37KM = 0b100101,
  DISTANCE_34KM = 0b100010,
  DISTANCE_31KM = 0b011111,
  DISTANCE_27KM = 0b011011,
  DISTANCE_24KM = 0b011000,
  DISTANCE_20KM = 0b010100,
  DISTANCE_17KM = 0b010001,
  DISTANCE_14KM = 0b001110,
  DISTANCE_12KM = 0b001100,
  DISTANCE_10KM = 0b001010,
  DISTANCE_8KM = 0b001000,
  DISTANCE_6KM = 0b000110,
  DISTANCE_5KM = 0b000101,
  STORM_IS_OVERHEAD = 0b000001
};

inline const char *distanceToString(std::uint8_t distance)
{
  switch (distance)
  {
    case StormFrontDistance::OUT_OF_RANGE:
      return "out of range";
    case StormFrontDistance::DISTANCE_40KM:
      return "40 km away";
    case StormFrontDistance::DISTANCE_37KM:
      return "37 km away";
    case StormFrontDistance::DISTANCE_34KM:
      return "34 km away";
    case StormFrontDistance::DISTANCE_31KM:
      return "31 km away";
    case StormFrontDistance::DISTANCE_27KM:
      return "27 km away";
    case StormFrontDistance::DISTANCE_24KM:
      return "24 km away";
    case StormFrontDistance::DISTANCE_20KM:
      return "20 km away";
    case StormFrontDistance::DISTANCE_17KM:
      return "17 km away";
    case StormFrontDistance::DISTANCE_14KM:
      return "14 km away";
    case StormFrontDistance::DISTANCE_12KM:
      return "12 km away";
    case StormFrontDistance::DISTANCE_10KM:
      return "10 km away";
    case StormFrontDistance::DISTANCE_8KM:
      return "8 km away";
    case StormFrontDistance::DISTANCE_6KM:
      return "6 km away";
    case StormFrontDistance::DISTANCE_5KM:
      return "5 km away";
    case StormFrontDistance::STORM_IS_OVERHEAD:
      return "overhead";
    default:
      return "invalid";
  }
}

#endif
