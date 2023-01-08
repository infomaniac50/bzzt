#include <cstdint>
#include <SPI.h>
#include <Wire.h>
#include "SparkFun_AS3935.h"
#include "LightningSensor.h"

// 0x03 is default, but the address can also be 0x02, or 0x01.
// Adjust the address jumpers on the underside of the product.
#define AS3935_ADDR 0x03

// Interrupt pin for lightning detection
// enable interrupt HS1_DATA6/VSPICS0/GPIO5/SCK
#define INTERRUPT_PIN 5

// If you're using I-squared-C then keep the following line. Address is set to default.
SparkFun_AS3935 lightning(AS3935_ADDR);

// interrupt trigger global var
volatile std::int8_t AS3935_ISR_Trig = 0;

// this is irq handler for AS3935 interrupts, has to return void and take no arguments
// always make code in interrupt handlers fast and short
void ARDUINO_ISR_ATTR AS3935_ISR()
{
    AS3935_ISR_Trig = 1;
}

void LightningSensor::attachInterruptPin()
{
    // When lightning is detected the interrupt pin goes HIGH.
    AS3935_ISR_Trig = 0; // clear trigger
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), AS3935_ISR, RISING);
}

void LightningSensor::detachInterruptPin()
{
    detachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN));
}

void LightningSensor::begin(SensorSettings sensorSettings)
{
    Wire.begin(); // Begin Wire before lightning sensor.
    if (!lightning.begin())
    { // Initialize the sensor.
        while (1)
            ;
    }

    setSensorSettings(sensorSettings);
    attachInterruptPin();
}

void LightningSensor::setSensorSettings(SensorSettings sensorSettings)
{
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


    lightning.calibrateOsc();

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
}

std::int8_t LightningSensor::getSensorEvent(SensorEvent *sensorEvent)
{
    std::uint8_t interrupted = AS3935_ISR_Trig;
    AS3935_ISR_Trig = 0;

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

    return interrupted;
}
