/***********************************************************************************
 * CanAirIO M5CoreInk
 * @author @hpsaturn
 * 
 * This project using CanAirIO Sensors Library. You can find the library here:
 * https://github.com/kike-canaries/canairio_sensorlib
 * 
 * The source code, documentation and the last version of this project here:
 * https://github.com/hpsaturn/co2_m5coreink
 * 
 * Tested with:
 * 
 * - One SCD30 (C02 sensor)
 * - One GCJA5 (Particulate Matter sensor)
 * - ENVII M5 Hat
 * 
 * But you can use it with any other i2c sensors, for example SPS30 or SCD41
 * UART sensors right nos is untested. 
 * 
 ***********************************************************************************/


#include <Arduino.h>
#include <M5CoreInk.h>
#include <Sensors.hpp>

#define DEEP_SLEEP_MODE       1     // eInk and esp32 hibernate support (recommended)
#define DEEP_SLEEP_TIME     600     // *** !! Please change it !! *** to 600s (10m) or more 
#define BEEP_ENABLE           1     // Eneble AQI high level alarm:
#define PM25_ALARM_BEEP      50     // PM2.5 level to trigger alarm
#define CO2_ALARM_BEEP     2000     // CO2 ppm level to trigger alarm

#define MAX_INIT_RETRY        3     // max retry count for i2c sensors
#define ENABLE_GxEPD2_GFX     0

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

UNIT mainUnit = UNIT::NUNIT;

uint32_t samples_count = 0;

bool drawReady;
bool isCharging;
bool isCalibrating;
bool onContinuousMode;

int initRetry = 0;


/************************************************
 *       eINK update methods
 ************************************************/

void displayMainValue(UNIT unit){
    uint16_t x = 15;
    uint16_t y = display.height() / 2 - 50;
    display.setTextSize(2);
    display.setCursor(x, y);
    display.setFont(&FreeMonoBold18pt7b);
    display.printf("%04i", (uint16_t)sensors.getUnitValue(unit));
    display.setFont(&FreeMonoBold9pt7b);
    String mainUnitSymbol = sensors.getUnitName(unit)+" / "+sensors.getUnitSymbol(unit);
    uint16_t lenght = mainUnitSymbol.length();
    x = (display.width() / 2) - ((lenght*11)/2);
    y = display.height() / 2 - 28;
    display.setTextSize(0);
    display.setCursor(x, y);
    display.print(mainUnitSymbol);
}

void displayValuesCallback(const void*) {
    display.fillScreen(GxEPD_WHITE);
    uint16_t x = 3;
    uint16_t y = display.height() / 2;
    display.writeLine(0,y-18,display.width(),y-18,GxEPD_BLACK);
    UNIT unit = sensors.getNextUnit();
    while (unit != UNIT::NUNIT) {
        String uName = sensors.getUnitName(unit);
        float uValue = sensors.getUnitValue(unit);
        String uSymb = sensors.getUnitSymbol(unit);
        Serial.println("-->[MAIN] process unit " + uName + " \t: " + String(uValue) + " " + uSymb);

        if ((unit == UNIT::CO2 || unit == UNIT::PM25) && mainUnit == UNIT::NUNIT) {
            mainUnit = unit;
        } else if (unit == UNIT::CO2 && mainUnit == UNIT::PM25) {
            mainUnit = unit;
        }
        /// Minor units list
        if (unit != mainUnit) {
            display.setFont(&FreeMonoBold9pt7b);
            display.setCursor(x, y);
            display.setTextSize(0);
            String minorName = sensors.getUnitName(unit);
            minorName.replace(".", "");
            minorName = minorName.substring(0, 4);
            String minorSymb = sensors.getUnitSymbol(unit).substring(0, 3);
            minorSymb.replace("/", "");
            String minorVal = String(sensors.getUnitValue(unit));
            display.printf("%4s: %7s %3s", minorName.c_str(), minorVal.c_str(), minorSymb.c_str());
            y = y + 18;
        }
        unit = sensors.getNextUnit();
    }

    if (mainUnit == UNIT::NUNIT)
        displayMainValue(UNIT::TEMP);  // if no main unit, display temperature
    else
        displayMainValue(mainUnit);

    delay(100);
    drawReady = true;
}

/************************************************
 *           eINK static GUI methods
 ************************************************/

void displayHomeCallback(const void*) {
    uint16_t x = 15;
    uint16_t y = display.height() / 2 - 30;
    display.fillScreen(GxEPD_WHITE);
    display.setFont(&FreeMonoBold18pt7b);
    display.setTextSize(1);
    display.setCursor(x, y);
    display.print("CanAirIO");
    display.setFont(&FreeMonoBold12pt7b);
    display.setTextSize(1);
    display.setCursor(x=40, y+30);
    display.print("M5CoreInk");
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextSize(0);
    String version = String(VERSION) + "." + String(REVISION);
    uint16_t lenght = version.length();
    x = (display.width() / 2) - ((lenght*11)/2);
    display.setCursor(x, y+50);
    display.print(version);
}

void calibrationReadyCallBack(const void*) {
    uint16_t x = 10;
    uint16_t y = display.height() / 2 - 20;
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print("!!CALIBRATED!!");
}

void loadingCallBack(const void*) {
    display.setCursor(10, 190);
    display.setTextSize(0);
    display.print("Restarting..");
}

void calibrationTitleCallback(const void*) {
    displayMainValue(UNIT::CO2);
    uint16_t x = 10;
    uint16_t y = display.height() / 2 + 25;
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextSize(1);
    display.setCursor(x, y);
    display.print("CO2 CALIBRATION:");
    
    display.setTextSize(1);
    display.setCursor(x, y+30);
    display.setTextSize(0);
    display.print("Send command?");

    display.setCursor(x, y + 50);
    display.setTextSize(0);
    display.print("CENTER:    YES");

    display.setCursor(x, y + 70);
    display.setTextSize(0);
    display.print("DOWN  :    NO");
    drawReady = true;
}

/************************************************
 *       GUI and utils methods
 ************************************************/

void displayPartialMode(void (*drawCallback)(const void*)) {
    static uint32_t timeStamp = 0;      
    if ((millis() - timeStamp > 1000)) {  // eInk refresh every 2 seconds
        timeStamp = millis();
        Serial.println("-->[eINK] displayValuesPartialMode..");
        drawReady = false;
        display.setPartialWindow(0, 0, display.width(), display.height());
        display.setRotation(0);
        display.setTextColor(GxEPD_BLACK);
        display.drawPaged(drawCallback, 0);
    }
}

void displayFullWindow(void (*drawCallback)(const void*)) {
    display.setRotation(0);
    display.setTextColor(GxEPD_BLACK);
    display.setFullWindow();
    display.drawPaged(drawCallback, 0);
}

void displayMessageTitle(void (*drawCallback)(const void*)) {
    display.setRotation(0);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setFullWindow();
    display.drawPaged(drawCallback, 0);
}

void beep(uint16_t freq = 2000, uint16_t dur = 50, uint16_t repeat = 1) {
    for (int i=0; i<repeat; i++) {
        M5.Speaker.tone(freq, dur);
        delay(dur*10);
    }
    M5.Speaker.mute();
}

void enableLED() {
    pinMode(LED_EXT_PIN, OUTPUT);
    digitalWrite(LED_EXT_PIN, LOW);
}

void disableLED() {
    pinMode(LED_EXT_PIN, OUTPUT);
    digitalWrite(LED_EXT_PIN, HIGH);
}

/************************************************
 *           Sensors methods
 * **********************************************/

/// for connection and disconnection demo
void resetVariables() {  
    mainUnit = UNIT::NUNIT;
    sensors.resetUnitsRegister();
    sensors.resetSensorsRegister();
    sensors.resetAllVariables();
}

void checkAQIAlarm() {
    uint16_t alarmValue = 0;
    uint16_t mainValue = sensors.getUnitValue(mainUnit);
    if (mainUnit == UNIT::PM25)
        alarmValue = PM25_ALARM_BEEP;
    else
        alarmValue = CO2_ALARM_BEEP;
    if (BEEP_ENABLE == 1 && mainValue > alarmValue) beep();
}

void shutdown() {
    if (DEEP_SLEEP_MODE == 1) {
        Serial.println("-->[LOOP] Deep sleep..");
        display.display(isCharging);
        display.powerOff();
        M5.shutdown(DEEP_SLEEP_TIME);
        Serial.println("-->[LOOP] USB is connected..");
        isCharging = true;  // it only is reached when the USB is connected
        Serial.println("-->[LOOP] Deep sleep done.");
    }
}

/// sensors callback, here we can get the values
void onSensorDataOk() { 
    if (isCalibrating) return;
    
    int max_samples_count = 2;                                        // number of samples before showing the values
    if (sensors.isUnitRegistered(UNIT::PM25)) max_samples_count = 7;  // if PM25 is registered, we need more samples
    
    Serial.println("-->[MAIN] read sensor attemp\t: " + String(samples_count) + "/" + String(max_samples_count));
    if (samples_count++ >= max_samples_count) {
        checkAQIAlarm();
        samples_count = 0;
        displayPartialMode(displayValuesCallback);
    }
    if (drawReady) {
        drawReady = false;
        resetVariables();
        if(!onContinuousMode) shutdown();
        else sensors.init();                        // Only for demostration we don't need to init again
    }
}

void onSensorNoData(const char * msg) {
    Serial.println(msg); 
    delay(500);
    /// wait for slow sensors like SCD30
    if (!sensors.isDataReady() && initRetry++<=MAX_INIT_RETRY) return;
    else if( initRetry>=MAX_INIT_RETRY) {
        resetVariables();
        displayPartialMode(displayValuesCallback);
        initRetry = 0;
    }
    if (drawReady && !onContinuousMode) {
        shutdown();        
        drawReady = false; 
    }
}

void checkCalibrationButton() {
    if (M5.BtnUP.isPressed()) {
        displayFullWindow(displayHomeCallback);
        isCalibrating = true;
        enableLED();
        while (!M5.BtnMID.isPressed()) {
            M5.update();
            sensors.loop();
            displayPartialMode(calibrationTitleCallback);
            if (M5.BtnMID.wasPressed()) {
                beep(2000, 30, 2);
                sensors.setCO2RecalibrationFactor(400);
                displayMessageTitle(calibrationReadyCallBack);
                break;
            }
            if (M5.BtnDOWN.wasPressed()) {
                displayMessageTitle(loadingCallBack);
                break;
            }
        }
        isCalibrating = false;
        samples_count = 6; // force refresh
        disableLED();
    }
}

void checkContinousModeButton() {
    if (M5.BtnDOWN.isPressed()) {
        enableLED();
        displayFullWindow(displayHomeCallback);
        onContinuousMode = true;
        while (!M5.BtnMID.isPressed()) {
            M5.update();
            sensors.loop();
        }
        onContinuousMode = false;
        disableLED();
    }
}

void checkClearScreenButton() {
    if (M5.BtnMID.isPressed()) {
        displayFullWindow(displayHomeCallback);
        sensors.readAllSensors();
        beep();
        displayPartialMode(displayValuesCallback);
    }
}

void checkButtons() {
    M5.update();
    checkClearScreenButton();    // mid button on boot
    checkContinousModeButton();  // down button on boot
    checkCalibrationButton();    // up button on boot
}

void sensorsInit() {
    Serial.println("-->[SETUP] Detecting sensors..");
    Wire.begin(32, 33);                          // I2C external port (bottom connector)
    Wire1.begin(25, 26);                         // I2C via Hat (top connector)
    sensors.setSampleTime(2);                    // config sensors sample time interval
    sensors.setSeaLevelPressure(1019.0);         // config sea level pressure (default: 1013.25)
    sensors.setOnDataCallBack(&onSensorDataOk);  // all data read callback
    sensors.setOnErrorCallBack(&onSensorNoData); // [optional] error callback
    sensors.setDebugMode(false);                 // [optional] debug mode
    sensors.detectI2COnly(true);                 // force to only i2c sensors
    sensors.init();                              // Auto detection to UART and i2c sensors

    if (sensors.isSensorRegistered(SENSORS::SSCD30)) {
        sensors.scd30.setMeasurementInterval(2);
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println();
    disableLED();
    sensorsInit();
    M5.begin(false, false, true);
    display.init(115200,false);
    checkButtons();
    Serial.println("-->[SETUP] setup done");
}

void loop() {
    sensors.loop();
}
