# M5CoreInk Multi Sensor

This a complete project that use the [CanAirIO Sensors Library](https://github.com/kike-canaries/canairio_sensorlib) to have a complete air quality device, with many posibilities and sensors, for example one CO2 sensor or the combination of it with particulate sensors (PM2.5) or environment sensors (T, H, P, Alt etc) with automatic selection of each driver and library for handling these sensors with a few lines of code. Plese see the [demo video](#demo)

Also the installation of this software in your device is possible only with one click via an easy [Web installer](#installation).


# Features

- [x] Multi variable multi sensor support with zero config
- [x] Partial update (fast refresh without blink) in USB mode
- [x] 5 sec measure rate in USB mode
- [x] configurable measure rate in Battery mode
- [x] Low power consumption in Battery mode (Deep sleep via RTC)
- [x] Able to set beep alarm for PM2.5 or CO2
- [x] Redraw complete screen and first setup via midle button
- [ ] NTP sync via WiFi or Bluetooth
- [ ] Compatibility with [CanAirIO](https://canair.io) project 
- [ ] GATT server for notifications

# Installation

## Easy way via CanAirIO Web Installer


---


# 3D Printer CO2 Hat alternative

In this repo you able to download the 3DPrint files for making a CO2 hat for M5CoreInk/M5StickC, this use a Sensirion SCD30 sensor for get CO2 values.

<a href="https://youtu.be/im0LNFRrHCg" target="_blank"><img src="images/youtube.jpg"></a>

## Components

| Description  | Store link |
|----------------------------|-------------------------------|
| M5CoreInk    |[M5Stack](https://m5stack.com/collections/m5-core/products/m5stack-esp32-core-ink-development-kit1-54-elnk-display) |
| M5Stack Proto | [M5Stack](https://m5stack.com/collections/m5-core/products/m5stack-esp32-core-ink-development-kit1-54-elnk-display) |
| Sensirion SCD30 | [Mouser](https://eu.mouser.com/ProductDetail/Sensirion/SCD30/?qs=rrS6PyfT74fdywu4FxpYjQ%3D%3D)
|      |      |
| M5StickC (compatible) | [M5Stack](https://m5stack.com/collections/m5-core/products/stick-c)
| M5StickC Plus (compatible) | [M5Stack](https://m5stack.com/collections/m5-core/products/m5stickc-plus-esp32-pico-mini-iot-development-kit)
|      |      |

![M5coreInk CO2 hat](images/collage.jpg)

## 3D print Files

<a href="https://github.com/hpsaturn/co2_m5coreink/tree/master/box" target="_blank"><img src="images/3Dpreview.jpg"></a>

# Build your own firmware 

Alternative to compiling and inatall your own firmware via PlatformIO with your favourite IDE (i.e. VSCode). Follow [this](https://platformio.org/platformio-ide) instructions. Also, you may need to install [git](http://git-scm.com/) in your system.

## Clone the repo

```sh
git clone https://github.com/hpsaturn/co2_m5coreink.git
```
and after that:

### Via IDE (vscode)

Connect the M5CoreInk or compatible board via USB. In Windows 10, drivers are installed automatically. I guess with other OS will be automatically installed too.

Open cloned folder with your PlatformIO IDE and build & upload it. For details please see the [documentation](https://docs.platformio.org/en/latest/integration/ide/vscode.html#quick-start), but the process flow is more easy than Arduino IDE flow, but you can also import it to the Arduino IDE if you want.

![PlatformIO Build tool](https://docs.platformio.org/en/latest/_images/platformio-ide-vscode-toolbar.png)

### Via PlatformIO CLI

After the clone, enter to directory and you can build and upload the current firmware from CLI:

```shell
pio run --target upload
```

