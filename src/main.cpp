#include <Arduino.h>
#include <M5CoreInk.h>
#include <SparkFun_SCD30_Arduino_Library.h>  //Click here to get the library: http://librarymanager/All#SparkFun_SCD30
#include <Wire.h>
#include <StreamString.h>

#define DEEP_SLEEP_MODE       1     // eInk and esp32 hibernate
#define DEEP_SLEEP_TIME      60     // seconds
#define SAMPLES_COUNT         4     // samples before suspend
#define LOOP_DELAY            2     // seconds
#define DISABLE_LED                 // improve battery


// base class GxEPD2_GFX can be used to pass references or pointers to the display instance as parameter, uses ~1.2k more code
// enable or disable GxEPD2_GFX base class
#define ENABLE_GxEPD2_GFX 0

// uncomment next line to use class GFX of library GFX_Root instead of Adafruit_GFX
//#include <GFX.h>
// Note: if you use this with ENABLE_GxEPD2_GFX 1:
//       uncomment it in GxEPD2_GFX.h too, or add #include <GFX.h> before any #include <GxEPD2_GFX.h>

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <GxEPD2_7C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include "bitmaps/Bitmaps200x200.h"  // 1.54" b/w

GxEPD2_154_M09 medp = GxEPD2_154_M09(/*CS=D8*/ 9, /*DC=D3*/ 15, /*RST=D4*/ 0, /*BUSY=D2*/ 4);
GxEPD2_BW<GxEPD2_154_M09, GxEPD2_154_M09::HEIGHT> display(medp);  // GDEH0154D67

SCD30 airSensor;
uint16_t co2value = 0;
float co2temp, co2humi;
uint16_t count;
bool drawReady;
bool isCharging;

void helloWorldCallback(const void*) {
    uint16_t x = 15;
    uint16_t y = display.height() / 2 - 30;
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print("0000");
}

void helloWorld() {
    //Serial.println("helloWorld");
    display.setRotation(0);
    display.setFont(&FreeMonoBold18pt7b);
    display.setTextSize(2);
    display.setTextColor(GxEPD_BLACK);
    display.setFullWindow();
    display.drawPaged(helloWorldCallback, 0);
    //Serial.println("helloWorld done");
}

void helloFullScreenPartialModeCallback(const void*) {
    Serial.print("drawing..");
    uint16_t x = 15;
    uint16_t y = display.height() / 2 - 30;
    display.fillScreen(GxEPD_WHITE);
    display.setTextSize(2);
    display.setCursor(x, y);
    display.setFont(&FreeMonoBold18pt7b);
    display.printf("%04i",co2value);

    display.setFont(&FreeMonoBold9pt7b);
    x = display.width() / 2 - 14;
    y = display.height() / 2 - 8;
    display.setTextSize(0);
    display.setCursor(x, y);
    display.print("PPM");

    display.setFont(&FreeMonoBold12pt7b);
    display.setTextSize(1);

    x = 11;
    y = display.height() / 2 + 40;
    display.setCursor(x, y);
    display.printf("Temp: %.2fC",co2temp);

    y = display.height() / 2 + 60;
    display.setCursor(x, y);
    display.printf("Humi: %.2f%%",co2humi);

    delay(100);

    drawReady = true;
    Serial.println("done");

}

void helloFullScreenPartialMode() {
    //Serial.println("helloFullScreenPartialMode");
    drawReady = false;
    display.setPartialWindow(0, 0, display.width(), display.height());
    display.setRotation(0);
    display.setFont(&FreeMonoBold24pt7b);
    display.setTextSize(1);
    display.setTextColor(GxEPD_BLACK);
    display.drawPaged(helloFullScreenPartialModeCallback, 0);
    //Serial.println("helloFullScreenPartialMode done");
}

bool sensorsLoop() {
    if (airSensor.dataAvailable()) {

        co2value = airSensor.getCO2();
        co2temp = airSensor.getTemperature();
        co2humi = airSensor.getHumidity();

        Serial.print("co2(ppm):");
        Serial.print(airSensor.getCO2());
        Serial.print(" temp(C):");
        Serial.print(airSensor.getTemperature(), 1);
        Serial.print(" humidity(%):");
        Serial.print(airSensor.getHumidity(), 1);
        Serial.println();

        return true;
    } 
    return false;
}

void co2sensorSetInterval(uint16_t sec) {
    airSensor.sendCommand(0x0104); // stop continous mesaurements
    delay(100);
    while(!airSensor.setMeasurementInterval(sec));  //Change number of seconds between measurements: 2 to 1800 (30 minutes)
}

void co2sensorInit(bool calibrate = false) {
    Wire.begin(25, 26);
    if (airSensor.begin(Wire) == false) {
        Serial.println("Air sensor not detected. Please check wiring. Freezing...");
        while (1);
    }

    //Read altitude compensation value
    unsigned int altitude = airSensor.getAltitudeCompensation();
    Serial.print("Current altitude: ");
    Serial.print(altitude);
    Serial.println("m");

    //My desk is ~1600m above sealevel
    airSensor.setAltitudeCompensation(34);  //Set altitude of the sensor in m, stored in non-volatile memory of SCD30

    //Pressure in Boulder, CO is 24.65inHg or 834.74mBar
    airSensor.setAmbientPressure(999);  //Current ambient pressure in mBar: 700 to 1200, will overwrite altitude compensation

    //Read temperature offset
    float offset = airSensor.getTemperatureOffset();
    Serial.print("Current temp offset: ");
    Serial.print(offset, 2);
    Serial.println("C");
    // airSensor.setTemperatureOffset(5);

    if(calibrate) airSensor.setForcedRecalibrationFactor(400);
}

void runDemo() {
    // first update should be full refresh
    helloWorld();
    delay(1000);
    // partial refresh mode can be used to full screen,
    // effective if display panel hasFastPartialUpdate
    helloFullScreenPartialMode();
    delay(1000);
    // helloArduino();
}

void setup() {
    Serial.begin(115200);
    Serial.println();
    delay(10);
    Serial.println("setup");

#ifdef DISABLE_LED    // turnoff it for improve battery life
    pinMode(LED_EXT_PIN, OUTPUT);
    digitalWrite(LED_EXT_PIN, HIGH);   
#endif

    M5.begin(false, false, true);

    co2sensorInit();

    display.init(115200,false);

    M5.update();
    if (M5.BtnMID.isPressed()) {
        helloWorld();
        while(!sensorsLoop());
        helloFullScreenPartialMode();
    }
    //helloWorld();
    delay(100);
    Serial.println("setup done");
}

void loop() {
    if (sensorsLoop()) {
        count++;
        if (count == SAMPLES_COUNT) {
            helloFullScreenPartialMode();
            count = 0;
        }
    }

    if (drawReady) {
        if (DEEP_SLEEP_MODE == 1) {
            display.display(isCharging);
            display.powerOff();
            M5.shutdown(DEEP_SLEEP_TIME);
            delay(LOOP_DELAY * 1000);  // it only is reached when the USB is connected
            isCharging = true;
        }
        else {
            delay(LOOP_DELAY * 1000);
        }
    }
}