
#include <Arduino.h>
#include <ESP32WifiCLI.hpp>
#include <LightningSensor.h>
#include <PubSubClient.h>
#include <String.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <arduino-timer.h>
#include <StormFrontDistance.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#include <DateTime.h>

// The calibration value for my board is 88 picoFarads.
const int TUNING_CAPACITOR_DEFAULT = 88;
const bool REPORT_DISTURBER_DEFAULT = false;
const char* HOSTNAME_DEFAULT = "green-mile";
const String SYSTEM_HOSTNAME(HOSTNAME_DEFAULT);
const char* TZ_AMERICA_CHICAGO = "CST6CDT,M3.2.0,M11.1.0";
// Plucked from /usr/share/i18n/locales/en_US on a local linux box
// % Appropriate date and time representation (%c)
// d_t_fmt "%a %d %b %Y %r %Z"
const char* DEFAULT_DATETIME_FORMAT = "%a %d %b %Y %r %Z";


SensorSettings settings;
LightningSensor sensor;
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

Timer<> timer = timer_create_default(); // create a timer with default settings

#pragma region "Error Visibility"
bool toggleErrorLed(void *)
{
  static bool state = false;

  state = !state;
  neopixelWrite(LED_BUILTIN, state ? RGB_BRIGHTNESS : 0, 0, 0);

  return true;
}

void setErrorStatus(bool isErrored = true)
{
  static Timer<>::Task error_task = 0;

  if (isErrored)
  {
    if (error_task == 0)
    {
      error_task = timer.every(500, toggleErrorLed);
    }
  }
  else
  {
    if (error_task != 0)
    {
      timer.cancel(error_task);
      error_task = 0;
    }
  }
}
#pragma endregion

#pragma region "MQTT Management"
bool checkSensor(void *)
{
  SensorEvent event;
  auto interrupted = sensor.getSensorEvent(&event);

  if (interrupted)
  {
    if (mqtt.connect(SYSTEM_HOSTNAME.c_str()))
    {
      StaticJsonDocument<256> doc;

      doc["type"] = statusToString(event.type);
      doc["distance"] = distanceToString(event.distance);
      doc["energy"] = event.energy;
      int64_t time_us = (int64_t)event.timestamp.tv_sec * 1000000L + (int64_t)event.timestamp.tv_usec;
      doc["timestamp"] = time_us;

      mqtt.beginPublish("lightning/event", measureJson(doc), false);
      BufferingPrint bufferedClient(mqtt, 32);
      serializeJson(doc, bufferedClient);
      bufferedClient.flush();
      mqtt.endPublish();
      setErrorStatus(false);
    }
    else
    {
      setErrorStatus(true);
    }
  }

  return true; // repeat? true
}

void onPubSubCallback(char *topic, byte *payload, unsigned int length)
{
  if (strcmp(topic, "lightning/ping") == 0)
  {
    StringPrint stream;
    stream.printf("Cores: %d\n", ESP.getChipCores());
    stream.printf("Model: %s\n", ESP.getChipModel());
    stream.printf("Revision: %d\n", ESP.getChipRevision());

    mqtt.publish("lightning/pong", stream.str().c_str(), false);
  }
}
#pragma endregion

#pragma region "Wifi CLI Callbacks"
class mESP32WifiCLICallbacks : public ESP32WifiCLICallbacks
{
    void onWifiStatus(bool isConnected)
    {
      if (isConnected)
      {
        if (!mqtt.connected())
        {

          String broker = wcli.getString("BROKER_HOST", "");
          if (!broker.isEmpty())
          {
            mqtt.setServer(broker.c_str(), 1883);
            mqtt.setCallback(onPubSubCallback);
          }

          if (mqtt.connect(SYSTEM_HOSTNAME.c_str()) && mqtt.subscribe("lightning/ping"))
          {
            setErrorStatus(false);
          }
          else
          {
            setErrorStatus(true);
          }
        }

        neopixelWrite(LED_BUILTIN, 0, RGB_BRIGHTNESS, 0);
      }
      else
      {
        neopixelWrite(LED_BUILTIN, 0, 0, RGB_BRIGHTNESS);
      }
    }

    void onHelpShow()
    {

    }

    void onNewWifi(String ssid, String passw) { }
};

void reboot(char *args, Stream *response){
  wcli.shell->clear();
  ESP.restart();
}

void setBroker(char *args, Stream *response)
{
  String argVal = wcli.parseArgument(args);

  argVal.trim();
  
  if (argVal.length() == 0) {
    response->println("Missing argument <BROKER_HOST>");
    return;
  }

  wcli.setString("BROKER_HOST", argVal);
  response->println("\r\nMQTT broker set to " + argVal);
  response->println("Please reboot to apply the change.");
}

void printCurrentDate(char *args, Stream *response) {
  if (DateTime.isTimeValid()) {
    String currentTime = DateTime.format(DEFAULT_DATETIME_FORMAT);
    response->printf("The system clock is valid.\n%s\n", currentTime.c_str());
  } else {
    response->println("The system clock is not valid.");
  }

}

void clearStorage(char *args, Stream *response) {
  wcli.clearSettings();
}

void setSetting(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);

  String argName = operands.first();

  if (argName.isEmpty()) {
    response->println("Missing argument <name>");
    // noiseFloor
    // watchdogThreshold
    // spikeRejection = 1 - 11
    // lightningThreshold
    // tuningCapacitor
    // sensorLocation
    // reportDisturber = 1 or 0
    // displayOscillatorAntenna = 1 or 0
    return;
  }

  String argValue = operands.second();

  if (argName.equalsIgnoreCase("spikeRejection")) {
    if (argValue.isEmpty()) {
      response->println("Missing argument <value>");
      return;
    }

    int value = argValue.toInt();
    if (value < 1 || value > 11) {
      response->println("Invalid argument <value>: You must enter a number between 1 and 11.");
      return;
    }

    settings.spikeRejection = (uint8_t) value;

    SparkFun_AS3935 rawSensor = sensor.getSensor();
    rawSensor.spikeRejection(settings.spikeRejection);

    return;
  }

  if (argName.equalsIgnoreCase("reportDisturber") == 1) {
    if (argValue.isEmpty()) {
      response->println("Missing argument <value>");
      return;
    }

    int value = argValue.toInt();
    if (value < 0 || value > 1) {
      response->println("The value argument must be 1 or 0, indicating true or false respectively.");
      return;
    }

    settings.reportDisturber = (bool) value;

    SparkFun_AS3935 rawSensor = sensor.getSensor();
    rawSensor.maskDisturber(!settings.reportDisturber);

    return;
  }

  response->println("Setting name not recognized.");
}

void getSetting(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  
  String argName = operands.first();
  
  if (argName.isEmpty()) {
    response->println("Missing argument <name>");
    // noiseFloor
    // watchdogThreshold
    // spikeRejection = 1 - 11
    // lightningThreshold
    // tuningCapacitor
    // sensorLocation
    // reportDisturber = 1 or 0
    // displayOscillatorAntenna = 1 or 0
    return;
  }

  String argValue = operands.second();

  if (argName.equalsIgnoreCase("spikeRejection") == 1) {
    SparkFun_AS3935 rawSensor = sensor.getSensor();
    settings.spikeRejection = rawSensor.readSpikeRejection();

    response->printf("spikeRejection: %d\n", settings.spikeRejection);

    return;
  }

  if (argName.equalsIgnoreCase("reportDisturber") == 1) {
    SparkFun_AS3935 rawSensor = sensor.getSensor();

    settings.reportDisturber = !((bool) rawSensor.readMaskDisturber());

    response->printf("reportDisturber: %s\n", settings.reportDisturber ? "true" : "false");

    return;
  }

  response->println("Setting name not recognized.");
}

void displayOsc(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  
  String argValue = operands.first();

  if (argValue.isEmpty()) {
    response->println("Missing argument <value>");
    return;
  }

  int value = argValue.toInt();

  if (value < 0 || value > 1) {
    response->println("The value argument must be 1 or 0, indicating true or false respectively.");
    return;
  }

  String argOsc = operands.second();

  if (argOsc.isEmpty()) {
    response->println("Missing argument <osc>");
  }

  int osc = argOsc.toInt();

  if (osc < 1 || osc > 3) {
    response->println("The osc argument must be between 1 and 3");
  }

  if (value == 1) {
    sensor.detachInterruptPin();
  }

  SparkFun_AS3935 rawSensor = sensor.getSensor();
  rawSensor.displayOscillator((bool) value, osc);

  if (value == 0) {
    sensor.attachInterruptPin();
  }
}

#pragma endregion

void setup()
{
  neopixelWrite(LED_BUILTIN, 0, 0, 0);
  // Initialize serial and wait for port to open:
  Serial.begin(115200);

  // pinMode(BUTTON, INPUT);
  // pinMode(STATUS_LED, OUTPUT);

  WiFi.hostname(SYSTEM_HOSTNAME);

  Serial.flush(); // Only for showing the message on serial
  delay(1000);
  wcli.setCallback(new mESP32WifiCLICallbacks());

  // Enter your custom commands:
  wcli.add("broker", &setBroker, "\t<hostname> set the MQTT broker hostname");
  wcli.add("reboot", &reboot, "\tperform a ESP32 reboot");
  wcli.add("date", &printCurrentDate, "\tprint date and time from the system clock");
  wcli.add("clear", &clearStorage, "\tClear non-volatile storage.");
  wcli.add("set", &setSetting, "\t<name> <value> Set lightning sensor setting.");
  wcli.add("get", &getSetting, "\t<name> Get lightning sensor setting.");
  wcli.add("displayOsc", &displayOsc, "\t<value> <osc>");

  wcli.shell->clear();
  // Connect to WPA/WPA2 network
  wcli.begin();
  
  DateTime.setTimeZone(TZ_AMERICA_CHICAGO);
  DateTime.begin();

  /*
    https://learn.adafruit.com/adafruit-esp32-feather-v2/pinouts#stemma-qt-connector-3112257

    At the top-left corner of the ESP32 module, is a STEMMA QT connector, labeled QT I2C on the silk.
    This connector allows you to connect a variety of sensors and breakouts with STEMMA QT connectors using various associated cables.

    You must enable the NEOPIXEL_I2C_POWER pin (GPIO 2) for the STEMMA QT connector power to work. Set it to be an output and HIGH in your code.

    There is a NEOPIXEL_I2C_POWER (GPIO 2) pin that must be set to an output and HIGH for the STEMMA QT connector power to work.
    For running in low power mode, you can disable (set output and LOW) the NEOPIXEL_I2C_POWER pin,
    this will turn off the separate 3.3V regulator that powers the QT connector's red wire
  */

  settings.tuningCapacitor = TUNING_CAPACITOR_DEFAULT;
  settings.reportDisturber = REPORT_DISTURBER_DEFAULT;

  if (!sensor.begin(settings)) {
    setErrorStatus(true);
  }

  // call the checkSensor function every 500 millis (0.5 second)
  // There is a one second window of time to read the interrupt register
  // after lightning is detected, and 1.5 after a disturber.
  timer.every(500, checkSensor);
}

void loop()
{
  mqtt.loop();
  timer.tick();
  wcli.loop();
}
