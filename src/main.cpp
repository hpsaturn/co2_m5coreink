#include <Arduino.h>
#include <M5CoreInk.h>
#include <Wire.h>
#include <StreamString.h>
#include <SparkFun_SCD30_Arduino_Library.h>  //Click here to get the library: http://librarymanager/All#SparkFun_SCD30

#define DEEP_SLEEP_MODE       1     // eInk and esp32 hibernate
#define DEEP_SLEEP_TIME     240     // seconds
#define SAMPLES_COUNT         2     // samples before suspend
#define LOOP_DELAY            2     // seconds
#define BEEP_ENABLE           1     // eneble high level alarm
#define ALARM_BEEP_VALUE   2500     // ppm level to trigger alarm
#define DISABLE_LED                 // improve battery

#define ENABLE_GxEPD2_GFX 0

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

void displayHomeCallback(const void*) {
    uint16_t x = 15;
    uint16_t y = display.height() / 2 - 30;
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print("0000");
}

void displayHome() {
    display.setRotation(0);
    display.setFont(&FreeMonoBold18pt7b);
    display.setTextSize(2);
    display.setTextColor(GxEPD_BLACK);
    display.setFullWindow();
    display.drawPaged(displayHomeCallback, 0);
}

void displayCO2ValuesCallback(const void*) {
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

/**
 * Display CO2 values in partialMode update.
 *  
 * it is a partial refresh mode can be used to full screen,
 * effective if display panel hasFastPartialUpdate
 */
void displayCO2ValuesPartialMode() {
    Serial.println("displayCO2ValuesPartialMode");
    Serial.print("drawing..");
    drawReady = false;
    display.setPartialWindow(0, 0, display.width(), display.height());
    display.setRotation(0);
    display.setFont(&FreeMonoBold24pt7b);
    display.setTextSize(1);
    display.setTextColor(GxEPD_BLACK);
    display.drawPaged(displayCO2ValuesCallback, 0);
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

void co2sensorConfig(bool calibrate = false) {

    //Read altitude compensation value
    unsigned int altitude = airSensor.getAltitudeCompensation();
    Serial.print("Current altitude: ");
    Serial.print(altitude);
    Serial.println("m");

    //Read temperature offset
    float offset = airSensor.getTemperatureOffset();
    Serial.print("Current temp offset: ");
    Serial.print(offset, 2);
    Serial.println("C");
    
    //Pressure in Boulder, CO is 24.65inHg or 834.74mBar
    while(!airSensor.setAmbientPressure(999));  //Current ambient pressure in mBar: 700 to 1200, will overwrite altitude compensation

    if(calibrate) {
        while(!airSensor.setAltitudeCompensation(34));  // Set altitude of the sensor in m, stored in non-volatile memory
        while(!airSensor.setTemperatureOffset(5));
        // while(!airSensor.setAutoSelfCalibration(false));
        // while(!airSensor.setForcedRecalibrationFactor(400));
    }
}

void co2sensorInit() {
    Wire.begin(25, 26);
    if (airSensor.begin(Wire) == false) {
        Serial.println("Air sensor not detected. Please check wiring. Freezing...");
        while (1);
    }
}

void beep() {
    M5.Speaker.tone(2000, 50);
    delay(30);
    M5.Speaker.mute();
}

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("setup init");

#ifdef DISABLE_LED    // turnoff it for improve battery life
    pinMode(LED_EXT_PIN, OUTPUT);
    digitalWrite(LED_EXT_PIN, HIGH);   
#endif
    
    co2sensorInit();

    M5.begin(false, false, true);
    display.init(115200,false);

    M5.update();
    if (M5.BtnMID.isPressed()) {
        co2sensorConfig();
        displayHome();
        while(!sensorsLoop());
        displayCO2ValuesPartialMode();
    }

    delay(100);
    Serial.println("setup done");
}

void loop() {
    if (sensorsLoop()) {
        count++;
        if (count == SAMPLES_COUNT) {
            displayCO2ValuesPartialMode();
            if(BEEP_ENABLE == 1 && co2value > ALARM_BEEP_VALUE ) beep();
            count = 0;
        }
    }

    if (drawReady) {
        if (DEEP_SLEEP_MODE == 1) {
            Serial.println("Deep sleep..");
            display.display(isCharging);
            display.powerOff();
            M5.shutdown(DEEP_SLEEP_TIME);
            Serial.println("USB is connected..");
            isCharging = true;              // it only is reached when the USB is connected
            Serial.println("Deep sleep done.");
        }
        drawReady = false;
    }

    delay(LOOP_DELAY * 1000);
}