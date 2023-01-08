# Green Mile

Publish MQTT messages about thunderstorm lightning in your area using an ESP32 and a Digital Lightning Sensor AS3935 from www.playingwithfusion.com

## Setup

1. Change the following values in main.cpp. The value for the tuning capacitor needs to match your board. The hostname can be whatever you want it to be. If you add it to your DNS server the device will be addressable by hostname.
   
   ```cpp
   #define TUNING_CAPACITOR_DEFAULT 88
   #define REPORT_DISTURBER_DEFAULT false
   #define HOSTNAME_DEFAULT "green-mile"
   ```
2. Upload the sketch to your board.
3. Open the serial terminal and update your settings. The program uses [ESP32WifiCLI - A Basic CLI for WiFi setup over ESP32](https://github.com/hpsaturn/esp32-wifi-cli) so you don't commit sensitive data to GitHub. (Type ``help`` in the terminal if you get stuck)
   1. Set the WiFi SSID ``setSSID "YOUR SSID"`` (You must include the quotes)
   2. Set the WiFi password ``setPASW "YOUR PASW"`` (You must include the quotes)
   3. Save the WiFi settings ``connect``
   4. Set the MQTT broker hostname ``broker <hostname>`` (No quotes are required)
   5. Reset the device by pressing the reset button
4. After configuration you should move it away from you computer and other noisy RFI producing devices. You can plug it in to any standard USB phone charger or power supply that the ESP32 accepts. The user LED will light up when the sketch connects to your MQTT server.
5. Publish a topic with the name ``lightning/ping`` and no payload. If the device is setup properly it will respond with a topic named ``lightning/pong`` and a payload with the number of cores in the CPU and the ESP32 model.

You know what all this means. *Means you're gonna ride the lightning.*

## Parts

### Lightning Sensor

[Qwiic Digital Lightning Sensor AS3935 SPI and I2C Breakout Kit](https://www.playingwithfusion.com/productview.php?pdid=135&catid=1012)

**Product Description**
Breakout board for the AS3935 digital lightning sensor based on the AMS reference design. Includes specially tuned antenna, SPI or I2C interfacing, and a wide 2.4V to 5.5V standard operating range. This innovative sensor is designed to interface with most current development systems and boards, including all current Arduino modules. The breakout board features an inductor (antenna) specially designed for this application, and the board ships fully calibrated. This ensures that you don’t have to write a massive back-end to support low-level IC calibration, just focus on your final application!

We store calibration values for each board shipped. The calibration value (in pF) is written on the lower corner of the product label. This information can also be provided at your request by contacting Technical Support and referencing your original order number.

### ESP32

[Adafruit ESP32 Feather V2](https://www.adafruit.com/product/5400)

**Product Description**

One of our star Feathers is the [Adafruit HUZZAH32 ESP32 Feather](https://www.adafruit.com/product/3405) - with the fabulous ESP32 WROOM module on there, it makes quick work of WiFi and Bluetooth projects that take advantage of Espressifs most popular chipset. Recently we had to redesign this feather to move from the obsolete CP2104 to the available CH9102F and one thing led to another and before you know it we made a completely refreshed design: the **Adafruit ESP32 Feather V2**.

**Features:**

- **ESP32 Dual core 240MHz Xtensa®** **processor** - the classic dual-core ESP32 you know and love!
- **Mini module** has FCC/CE certification and comes with 8 MByte of Flash and 2 MByte of PSRAM - you can have huge data buffers
- **Power options** - USB type C **or** Lipoly battery
- **Built-in battery charging** when powered over USB-C
- **LiPoly battery monitor** with two 200K resistor divider
- **Reset and User** (I38) buttons to reset board and as a separate input
- **High speed upload with auto-reset and serial debug** with ultra-reliable CP2102N chipset.
- **STEMMA QT** connector for I2C devices, with switchable power, so you can go into low power mode.
- **Charge/User** LEDs + status **NeoPixel** with pin-controlled power for low power usage
- **Low Power friendly**! In deep sleep mode we can get down to 80~100uA of current draw from the Lipoly connection. Quiescent current is from the power regulator, ESP32 chip, and Lipoly monitor. Turn off the NeoPixel and external I2C power for the lowest quiescent current draw.
- **Works with Arduino or MicroPython**