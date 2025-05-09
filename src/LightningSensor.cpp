#include <cstdint>
#include <SPI.h>
#include <Wire.h>
#include "SparkFun_AS3935.h"
#include "LightningSensor.h"
#include <sys/time.h>

/*
  Qwiic Digital Lightning Sensor AS3935 SPI and I2C Breakout Kit
  https://www.playingwithfusion.com/productview.php?pdid=135&catid=1012

  **Product Description**
  Breakout board for the AS3935 digital lightning sensor based on the AMS reference design. Includes specially tuned antenna, SPI or I2C interfacing, and a wide 2.4V to 5.5V standard operating range. This innovative sensor is designed to interface with most current development systems and boards, including all current Arduino modules. The breakout board features an inductor (antenna) specially designed for this application, and the board ships fully calibrated. This ensures that you don’t have to write a massive back-end to support low-level IC calibration, just focus on your final application!

  We store calibration values for each board shipped. The calibration value (in pF) is written on the lower corner of the product label. This information can also be provided at your request by contacting Technical Support and referencing your original order number.
*/

// 0x03 is default, but the address can also be 0x02, or 0x01.
// Adjust the address jumpers on the underside of the product.
const int AS3935_ADDR = 0x03;

// Interrupt pin for lightning detection
// enable interrupt D14/GPIO14/ADC2_CH6/HSPICLK/HS2_CLK
const int INTERRUPT_PIN = 14;

// If you're using I-squared-C then keep the following line. Address is set to default.
SparkFun_AS3935 lightning(AS3935_ADDR);

// interrupt trigger global var
volatile bool sensorInterruptTriggered = false;

// this is irq handler for AS3935 interrupts, has to return void and take no arguments
// always make code in interrupt handlers fast and short
void ARDUINO_ISR_ATTR AS3935_ISR()
{
  sensorInterruptTriggered = true;
}

bool interruptAttached;

void LightningSensor::attachInterruptPin()
{
  if (!interruptAttached) {
    // When lightning is detected the interrupt pin goes HIGH.
    sensorInterruptTriggered = false; // clear trigger
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), AS3935_ISR, RISING);
    interruptAttached = true;
  }

}

void LightningSensor::detachInterruptPin()
{
  if (interruptAttached) {
    detachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN));
    interruptAttached = false;
  }
}

int LightningSensor::begin(SensorSettings sensorSettings, bool enableInterruptPin)
{
  Wire.begin(); // Begin Wire before lightning sensor.
  if (!lightning.begin())
  { // Initialize the sensor.
    return -1;
  }

  lightning.resetSettings();

  // "Disturbers" are events that are false lightning events. If you find
  // yourself seeing a lot of disturbers you can have the chip not report those
  // events on the interrupt lines.
  lightning.maskDisturber(!sensorSettings.reportDisturber);

  // The lightning detector defaults to an indoor setting (less
  // gain/sensitivity), if you plan on using this outdoors
  // uncomment the following line:
  lightning.setIndoorOutdoor(sensorSettings.sensorLocation);

  lightning.tuneCap(sensorSettings.tuningCapacitor);

  // Noise floor setting from 1-7, one being the lowest. Default setting is
  // two. If you need to check the setting, the corresponding function for
  // reading the function follows.
  lightning.setNoiseLevel(sensorSettings.noiseFloor);

  // Watchdog threshold setting can be from 1-10, one being the lowest. Default setting is
  // two. If you need to check the setting, the corresponding function for
  // reading the function follows.
  lightning.watchdogThreshold(sensorSettings.watchdogThreshold);

  // Spike Rejection setting from 1-11, one being the lowest. Default setting is
  // two. If you need to check the setting, the corresponding function for
  // reading the function follows.
  // The shape of the spike is analyzed during the chip's
  // validation routine. You can round this spike at the cost of sensitivity to
  // distant events.
  lightning.spikeRejection(sensorSettings.spikeRejection);

  // This setting will change when the lightning detector issues an interrupt.
  // For example you will only get an interrupt after five lightning strikes
  // instead of one. Default is one, and it takes settings of 1, 5, 9 and 16.
  // Followed by its corresponding read function. Default is zero.
  lightning.lightningThreshold(sensorSettings.lightningThreshold);


  bool calibrationSuccessful = lightning.wakeUp();

  // When the distance to the storm is estimated, it takes into account other
  // lightning that was sensed in the past 15 minutes. If you want to reset
  // time, then you can call this function.

  lightning.clearStatistics(true);

  // The power down function has a BIG "gotcha". When you wake up the board
  // after power down, the internal oscillators will be recalibrated. They are
  // recalibrated according to the resonance frequency of the antenna - which
  // should be around 500kHz. It's highly recommended that you calibrate your
  // antenna before using these two functions, or you run the risk of schewing
  // the timing of the chip.

  // lightning.powerDown();
  // delay(1000);
  // if( lightning.wakeUp() )
  //  Serial.println("Successfully woken up!");
  // else
  // Serial.println("Error recalibrating internal osciallator on wake up.");
  if (enableInterruptPin) {
    attachInterruptPin();
  }

  return calibrationSuccessful ? 0 : -2;
}

SparkFun_AS3935& LightningSensor::getSensor() {
  return lightning;
}

bool LightningSensor::isTriggered()
{
  return sensorInterruptTriggered;
}

void LightningSensor::getSensorEvent(SensorEvent *sensorEvent)
{
  sensorInterruptTriggered = false;

  struct timeval now;
  gettimeofday(&now, nullptr);
  sensorEvent->timestamp.tv_sec = now.tv_sec;
  sensorEvent->timestamp.tv_usec = now.tv_usec;

  // Hardware has alerted us to an event, now we read the interrupt register
  // to see exactly what it is.
  sensorEvent->type = lightning.readInterruptReg();

  if (sensorEvent->type == LIGHTNING)
  {
    // Lightning! Now how far away is it? Distance estimation takes into
    // account previously seen events.
    sensorEvent->distance = lightning.distanceToStorm();

    // "Lightning Energy" and I do place into quotes intentionally, is a pure
    // number that does not have any physical meaning.
    sensorEvent->energy = lightning.lightningEnergy();
  }
  else
  {
    // On any other event, these values are considered invalid.
    sensorEvent->distance = 0;
    sensorEvent->energy = 0;
  }
}
