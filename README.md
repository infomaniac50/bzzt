![logo.png](logo.png)

Publish MQTT messages about thunderstorm lightning in your area using an ESP32 and a Digital Lightning Sensor AS3935 from www.playingwithfusion.com

## Setup

1. Change the following values in main.cpp. The value for the tuning capacitor needs to match your sensor board. The hostname can be whatever you want it to be. If you add it to your DNS server the device will be addressable by hostname.
   
   ```cpp
   #define TUNING_CAPACITOR_DEFAULT 88
   #define REPORT_DISTURBER_DEFAULT false
   #define HOSTNAME_DEFAULT "bzzt"
   ```

2. Upload the sketch to your board.

3. Open the serial terminal and update your settings. The program uses [ESP32WifiCLI - A Basic CLI for WiFi setup over ESP32](https://github.com/hpsaturn/esp32-wifi-cli) so you don't commit sensitive data to GitHub. (Type ``help`` in the terminal if you get stuck)
   
   1. Set the WiFi SSID and password nmcli connect Your SSID password "Your Password"
   2. Set the MQTT broker hostname ``broker <hostname>`` *Do not use quotes here*
   3. Reset the device by pressing the reset button

4. After configuration you should move it away from your computer and other noisy RFI producing devices. You can plug it into any standard USB phone charger or power supply that the ESP32 accepts. The user LED will light up when the sketch connects to your MQTT server.

5. Publish a topic with the name ``lightning/ping`` and no payload. If the device is setup properly it will respond with a topic named ``lightning/pong`` and a payload with the number of cores in the CPU and the model number of the ESP32 daughter board.

After completing setup, you just have to wait for a storm to show up or buy a [Digital Lightning Sensor Tester Arduino Shield for AS3935](https://www.playingwithfusion.com/productview.php?pdid=55&catid=1012). The device will publish messages about stormfront distance and a unit-less *energy* metric to your MQTT server. You can consume those messages from another device and post them to Twitter or draw something on your Magic Mirror, etc.

*Onomatopoeia* (pronounced: "on-uh-mat-uh-PEE-uh")  

- A word that imitates or suggests the sound it represents.

*BZZT* - For electric buzz

## Parts

### Lightning Sensor

[Qwiic Digital Lightning Sensor AS3935 SPI and I2C Breakout Kit](https://www.playingwithfusion.com/productview.php?pdid=135&catid=1012)

**Product Description**
Breakout board for the AS3935 digital lightning sensor based on the AMS reference design. Includes specially tuned antenna, SPI or I2C interfacing, and a wide 2.4V to 5.5V standard operating range. This innovative sensor is designed to interface with most current development systems and boards, including all current Arduino modules. The breakout board features an inductor (antenna) specially designed for this application, and the board ships fully calibrated. This ensures that you don’t have to write a massive back-end to support low-level IC calibration, just focus on your final application!

We store calibration values for each board shipped. The calibration value (in pF) is written on the lower corner of the product label. This information can also be provided at your request by contacting Technical Support and referencing your original order number.

### ESP32

[ESP32-S3-DevKitC-1-N8R8 rev 1.1](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide.html)

The ESP32-S3-DevKitC-1 is an entry-level development board equipped with ESP32-S3-WROOM-1, ESP32-S3-WROOM-1U, or ESP32-S3-WROOM-2, a general-purpose Wi-Fi + Bluetooth® Low Energy MCU module that integrates complete Wi-Fi and Bluetooth Low Energy functions.

Most of the I/O pins on the module are broken out to the pin headers on both sides of this board for easy interfacing. Developers can either connect peripherals with jumper wires or mount ESP32-S3-DevKitC-1 on a breadboard.

**Component Note**

For boards with Octal SPI flash/PSRAM memory embedded ESP32-S3-WROOM-1/1U modules, and boards with ESP32-S3-WROOM-2 modules, the pins GPIO35, GPIO36 and GPIO37 are used for the internal communication between ESP32-S3 and SPI flash/PSRAM memory, thus not available for external use.

**Hardware Revision Note**

Both the initial and v1.1 versions of ESP32-S3-DevKitC-1 are available on the market. The main difference lies in the GPIO assignment for the RGB LED: the initial version uses GPIO48, whereas v1.1 uses GPIO38.

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