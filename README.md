# M5CoreInk / M5StickC CO2 Sensirion sensor hat

Source code and 3DPrint files for making a CO2 hat for M5CoreInk/M5StickC (or any ESP32 compatible board), this use a Sensirion SCD30 sensor for get CO2 values.

![M5coreInk CO2 hat](images/collage.jpg)

# Components

| Description  | Store link |
|----------------------------|-------------------------------|
| M5CoreInk    |[M5Stack](https://m5stack.com/collections/m5-core/products/m5stack-esp32-core-ink-development-kit1-54-elnk-display) |
| M5Stack Proto | [M5Stack](https://m5stack.com/collections/m5-core/products/m5stack-esp32-core-ink-development-kit1-54-elnk-display) |
| Sensirion SCD30 | [Mouser](https://eu.mouser.com/ProductDetail/Sensirion/SCD30/?qs=rrS6PyfT74fdywu4FxpYjQ%3D%3D)
|      |      |
| M5StickC (compatible) | [M5Stack](https://m5stack.com/collections/m5-core/products/stick-c)
| M5StickC Plus (compatible) | [M5Stack](https://m5stack.com/collections/m5-core/products/m5stickc-plus-esp32-pico-mini-iot-development-kit)
|      |      |

# How to build it

<a href="https://youtu.be/im0LNFRrHCg" target="_blank"><img src="images/youtube.jpg"></a>


# Firmware upload

First of all, install PlatformIO with your favourite IDE (i.e. VSCode). Follow [this](https://platformio.org/platformio-ide) instructions. Also, you may need to install [git](http://git-scm.com/) in your system.

## Clone the repo

```sh
git clone https://github.com/hpsaturn/co2_m5coreink.git
```

### Via IDE (vscode)

Connect the M5CoreInk or compatible board via USB. In Windows 10, drivers are installed automatically. I guess with other OS will be automatically installed too.

Open cloned folder with your PlatformIO IDE and build & upload it. For details please see the [documentation](https://docs.platformio.org/en/latest/integration/ide/vscode.html#quick-start), but the process flow is more easy than Arduino IDE flow, but you can also import it to the Arduino IDE if you want.

![PlatformIO Build tool](https://docs.platformio.org/en/latest/_images/platformio-ide-vscode-toolbar.png)

### Via PlatformIO CLI

After the clone, enter to directory and you can build and upload the current firmware from CLI:

```shell
pio run --target upload
```

# 3D print Files

<a href="https://github.com/hpsaturn/co2_m5coreink/tree/master/box" target="_blank"><img src="images/3Dpreview.jpg"></a>

