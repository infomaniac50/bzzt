
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

// The calibration value for my board is 88 picoFarads.
#define TUNING_CAPACITOR_DEFAULT 88
#define REPORT_DISTURBER_DEFAULT false
#define HOSTNAME_DEFAULT "green-mile"

SensorSettings settings;

/*
Qwiic Digital Lightning Sensor AS3935 SPI and I2C Breakout Kit
https://www.playingwithfusion.com/productview.php?pdid=135&catid=1012

**Product Description**
Breakout board for the AS3935 digital lightning sensor based on the AMS reference design. Includes specially tuned antenna, SPI or I2C interfacing, and a wide 2.4V to 5.5V standard operating range. This innovative sensor is designed to interface with most current development systems and boards, including all current Arduino modules. The breakout board features an inductor (antenna) specially designed for this application, and the board ships fully calibrated. This ensures that you don’t have to write a massive back-end to support low-level IC calibration, just focus on your final application!

We store calibration values for each board shipped. The calibration value (in pF) is written on the lower corner of the product label. This information can also be provided at your request by contacting Technical Support and referencing your original order number.
*/
LightningSensor sensor;

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

const String systemHostname(HOSTNAME_DEFAULT);

auto timer = timer_create_default(); // create a timer with default settings

#pragma region "Error Visibility"
bool toggleErrorLed(void *)
{
  static bool state = false;

  state = !state;
  digitalWrite(LED_BUILTIN, state);

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

  if (interrupted == 1)
  {
    if (mqtt.connect(systemHostname.c_str()))
    {
      StaticJsonDocument<256> doc;

      doc["type"] = statusToString(event.type);
      doc["distance"] = distanceToString(event.distance);
      doc["energy"] = event.energy;

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
    esp_chip_info_t info;
    esp_chip_info(&info);
    StringPrint stream;
    stream.print(F("Cores: "));
    stream.println(info.cores);
    stream.print(F("Model: "));
    switch (info.model)
    {
    case CHIP_ESP32:
      stream.println(F("ESP32"));
      break;
    case CHIP_ESP32S2:
      stream.println(F("ESP32-S2"));
      break;
    case CHIP_ESP32S3:
      stream.println(F("ESP32-S3"));
      break;
    case CHIP_ESP32C3:
      stream.println(F("ESP32-C3"));
      break;
    case CHIP_ESP32H2:
      stream.println(F("ESP32-H2"));
      break;
    default:
      stream.println(F("Unknown"));
      break;
    }

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

        if (mqtt.connect(systemHostname.c_str()) && mqtt.subscribe("lightning/ping"))
        {
          setErrorStatus(false);
        }
        else
        {
          setErrorStatus(true);
        }
      }

      digitalWrite(LED_BUILTIN, HIGH);
    }
    else
    {
      digitalWrite(LED_BUILTIN, LOW);
    }
  }

  void onHelpShow()
  {
    // Enter your custom help here:
    Serial.println("\r\nLightning Rider commands:\r\n");
    Serial.println("broker <hostname> \tset the MQTT broker hostname");
    Serial.println("reboot\t\t\tperform a soft ESP32 reboot");
  }

  void onNewWifi(String ssid, String passw) {
  }
};

void reboot(String opts)
{
  ESP.restart();
}

void setBroker(String opts)
{
  maschinendeck::Pair<String, String> operands = maschinendeck::SerialTerminal::ParseCommand(opts);
  String broker = operands.first();

  wcli.setString("BROKER_HOST", broker);
  Serial.println("\r\nMQTT broker set to " + broker);
  Serial.println("Please reboot to apply the change.");
}
#pragma endregion

void setup()
{
  // Initialize serial and wait for port to open:
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);

  WiFi.hostname(systemHostname);

  Serial.flush(); // Only for showing the message on serial
  delay(1000);
  wcli.setCallback(new mESP32WifiCLICallbacks());

  // Connect to WPA/WPA2 network
  wcli.begin();

  // attempt to connect to Wifi network:
  if (WiFi.status() != WL_CONNECTED)
  {
    delay(5000);
    wcli.connect();
  }

  // Enter your custom commands:
  wcli.term->add("broker", &setBroker, "\t<hostname> set the MQTT broker hostname");
  wcli.term->add("reboot", &reboot, "\tperform a ESP32 reboot");

  /*
  https://learn.adafruit.com/adafruit-esp32-feather-v2/pinouts#stemma-qt-connector-3112257

  At the top-left corner of the ESP32 module, is a STEMMA QT connector, labeled QT I2C on the silk.
  This connector allows you to connect a variety of sensors and breakouts with STEMMA QT connectors using various associated cables.

  You must enable the NEOPIXEL_I2C_POWER pin (GPIO 2) for the STEMMA QT connector power to work. Set it to be an output and HIGH in your code.

  There is a NEOPIXEL_I2C_POWER (GPIO 2) pin that must be set to an output and HIGH for the STEMMA QT connector power to work.
  For running in low power mode, you can disable (set output and LOW) the NEOPIXEL_I2C_POWER pin,
  this will turn off the separate 3.3V regulator that powers the QT connector's red wire
  */
  pinMode(NEOPIXEL_I2C_POWER, OUTPUT);
  digitalWrite(NEOPIXEL_I2C_POWER, HIGH);

  settings.tuningCapacitor = TUNING_CAPACITOR_DEFAULT;
  settings.reportDisturber = REPORT_DISTURBER_DEFAULT;

  sensor.begin(settings);

  // call the checkSensor function every 500 millis (0.5 second)
  timer.every(500, checkSensor);
}

void loop()
{
  mqtt.loop();
  timer.tick();
  wcli.loop();
}
