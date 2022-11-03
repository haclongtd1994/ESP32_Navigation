#include <Arduino.h>
#include "main.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <Button2.h> // version 2.0.3, available in Arduino libraries, or https://github.com/LennartHennigs/Button2
#include "DataConstants.h"
#include "ImagesDirections.h"
#include "imagesLogo.h"
#include "Display.h"
#include "Configuration.h"

/* --------------------------------------OlED--------------------------------------*/
// -----------------
// Display selection
// Uncomment wanted display with corresponding header
// -----------------

// OLED 128x128 RGB, Waveshare 14747, driver SSD1351
// Doesn't require external libraries
// Pins: DIN=23, CLK=18, CS=5, DC=17, RST=16, uses SPIClass(VSPI)

SSD1306Wire oled(0x3c, 27, 33); //SDA pin 27, SCL pin 33
Display IDisplay(&oled);

void DrawDirection(uint8_t direction)
{
    const uint8_t* imageProgmem = ImageFromDirection(direction);
    if (imageProgmem)
    {
        IDisplay.drawImage(0, 0, 64, 64, imageProgmem);
    }
}

void DrawSpeed(uint8_t speed)
{
    if (speed == 0)
        return;

    char str[7] = {};
    sprintf(str, "%u Km", (unsigned int)speed);

    IDisplay.drawMessage(128, 5, 128, str, ArialMT_Plain_16, TEXT_ALIGN_RIGHT);
}

const uint8_t* ImageFromDirection(uint8_t direction)
{
    switch (direction)
    {
        case DirectionNone: return nullptr;
        case DirectionStart: return IMG_directionWaypoint;
        case DirectionEasyLeft: return IMG_directionEasyLeft;
        case DirectionEasyRight: return IMG_directionEasyRight;
        case DirectionEnd: return IMG_directionWaypoint;
        case DirectionVia: return IMG_directionWaypoint;
        case DirectionKeepLeft: return IMG_directionKeepLeft;
        case DirectionKeepRight: return IMG_directionKeepRight;
        case DirectionLeft: return IMG_directionLeft;
        case DirectionOutOfRoute: return IMG_directionOutOfRoute;
        case DirectionRight: return IMG_directionRight;
        case DirectionSharpLeft: return IMG_directionSharpLeft;
        case DirectionSharpRight: return IMG_directionSharpRight;
        case DirectionStraight: return IMG_directionStraight;
        case DirectionUTurnLeft: return IMG_directionUTurnLeft;
        case DirectionUTurnRight: return IMG_directionUTurnRight;
        case DirectionFerry: return IMG_directionFerry;
        case DirectionStateBoundary: return IMG_directionStateBoundary;
        case DirectionFollow: return IMG_directionFollow;
        case DirectionMotorway: return IMG_directionMotorway;
        case DirectionTunnel: return IMG_directionTunnel;
        case DirectionExitLeft: return IMG_directionExitLeft;
        case DirectionExitRight: return IMG_directionExitRight;
        case DirectionRoundaboutRSE: return IMG_directionRoundaboutRSE;
        case DirectionRoundaboutRE: return IMG_directionRoundaboutRE;
        case DirectionRoundaboutRNE: return IMG_directionRoundaboutRNE;
        case DirectionRoundaboutRN: return IMG_directionRoundaboutRN;
        case DirectionRoundaboutRNW: return IMG_directionRoundaboutRNW;
        case DirectionRoundaboutRW: return IMG_directionRoundaboutRW;
        case DirectionRoundaboutRSW: return IMG_directionRoundaboutRSW;
        case DirectionRoundaboutRS: return IMG_directionRoundaboutRS;
        case DirectionRoundaboutLSE: return IMG_directionRoundaboutLSE;
        case DirectionRoundaboutLE: return IMG_directionRoundaboutLE;
        case DirectionRoundaboutLNE: return IMG_directionRoundaboutLNE;
        case DirectionRoundaboutLN: return IMG_directionRoundaboutLN;
        case DirectionRoundaboutLNW: return IMG_directionRoundaboutLNW;
        case DirectionRoundaboutLW: return IMG_directionRoundaboutLW;
        case DirectionRoundaboutLSW: return IMG_directionRoundaboutLSW;
        case DirectionRoundaboutLS: return IMG_directionRoundaboutLS;
    }
    return IMG_directionError;
}
/* --------------------------------------DeepSleep--------------------------------------*/

RTC_DATA_ATTR int bootCount = 0;
uint32_t g_currentTDeepsleep = 0;
uint32_t g_lastTDeepsleep = 0;
uint8_t g_displayDisSecond = 0;
char g_displayDisSecond_Char[64] = {};

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void print_wakeup_reason(void){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

/* --------------------------------------BLE--------------------------------------*/
// ---------------------
// Constants
// ---------------------
#define SERVICE_UUID        "DD3F0AD1-6239-4E1F-81F1-91F6C9F01D86"
#define CHAR_INDICATE_UUID  "DD3F0AD2-6239-4E1F-81F1-91F6C9F01D86"
#define CHAR_WRITE_UUID     "DD3F0AD3-6239-4E1F-81F1-91F6C9F01D86"

// -----------------
// Variables for BLE
// -----------------
BLEServer* g_pServer = NULL;
BLECharacteristic* g_pCharIndicate = NULL;
bool g_deviceConnected = false;
uint32_t g_lastActivityTime = 0;
bool g_isNaviDataUpdated = false;
std::string g_naviData;

// ---------------------
// Bluetooth event callbacks
// ---------------------
class MyServerCallbacks: public BLEServerCallbacks
{
    void onConnect(BLEServer* pServer) override
    {
        g_deviceConnected = true;
        g_currentTDeepsleep = 0;
        g_lastTDeepsleep = 0;
        g_lastActivityTime = millis();
    }

    void onDisconnect(BLEServer* pServer) override
    {
        g_deviceConnected = false;
        g_currentTDeepsleep = millis();
        BLEDevice::startAdvertising();
    }
};

class MyCharWriteCallbacks: public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        g_lastActivityTime = millis();
        std::string value = pCharacteristic->getValue();

        if (value.length() > 0)
        {
            g_naviData = value;
            g_isNaviDataUpdated = true;
            Serial.print("New value, length = ");
            Serial.print(value.length());
            Serial.print(": ");
            for (int i = 0; i < value.length(); ++i)
            {
                char tmp[4] = "";
                sprintf(tmp, "%02X ", value[i]);
                Serial.print(tmp);
            }
            Serial.println();
        }
    }
};

/* --------------------------------------Setup--------------------------------------*/
void setup()
{
    Serial.begin(115200);
    Serial.println("BLENaviPeripheral2 setup() started");

    /* ----------------- OLED --------------------- */
    Serial.println("Display init started");
	IDisplay.init();
	IDisplay.drawWelcomeScreen();
    Serial.println("Display init done");
    delay(3000);
	IDisplay.drawConnectionScreen((const uint8_t* )LOGO_Image_1);
    delay(3000);

    /* ----------------- BLE --------------------- */
    Serial.println("BLE init started");
    IDisplay.clearscreen();
    IDisplay.drawMessage(32, 0, 128, "LOG INITIAL", ArialMT_Plain_10, TEXT_ALIGN_CENTER);
    IDisplay.displayscreen();
    delay(50);
    IDisplay.drawMessage(0, 12, 128, "-- BLE Init Start --", ArialMT_Plain_10, TEXT_ALIGN_LEFT);
    IDisplay.displayscreen();
    delay(50);
    BLEDevice::init("ESP32 HUD");
    g_pServer = BLEDevice::createServer();
    g_pServer->setCallbacks(new MyServerCallbacks());
    BLEService *pService = g_pServer->createService(SERVICE_UUID);
    // characteristic for indicate
    {
        uint32_t charProperties = BLECharacteristic::PROPERTY_INDICATE;
        g_pCharIndicate = pService->createCharacteristic(CHAR_INDICATE_UUID, charProperties);
        g_pCharIndicate->addDescriptor(new BLE2902());
        g_pCharIndicate->setValue("");
    }
    // characteristic for write
    {
        uint32_t charProperties = BLECharacteristic::PROPERTY_WRITE;
        BLECharacteristic *pCharWrite = pService->createCharacteristic(CHAR_WRITE_UUID, charProperties);
        pCharWrite->setCallbacks(new MyCharWriteCallbacks());
    }
    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    Serial.println("BLE init done");
    IDisplay.drawMessage(0, 24, 128, "-- BLE Init Done --", ArialMT_Plain_10, TEXT_ALIGN_LEFT);
    IDisplay.displayscreen();
    delay(50);

    // setup deep sleep
    g_currentTDeepsleep = millis();
    g_displayDisSecond = 0;
    IDisplay.drawMessage(0, 32, 128, "-- Deepsleep Init Start --", ArialMT_Plain_10, TEXT_ALIGN_LEFT);
    IDisplay.displayscreen();
    delay(50);
    bootCount++;
    Serial.println("Boot number: " + String(bootCount));
    //Print the wakeup reason for ESP32
    print_wakeup_reason();
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_15, HIGH);

    Serial.println("setup() DONE ");
    IDisplay.drawMessage(0, 44, 128, " -- All setup Done --", ArialMT_Plain_10, TEXT_ALIGN_LEFT);
    IDisplay.displayscreen();
}

void loop()
{
    if (g_deviceConnected)
    {
        if (g_isNaviDataUpdated)
        {
            g_isNaviDataUpdated = false;

            std::string currentData = g_naviData;
            if (currentData.size() > 0)
            {
                if (currentData[0] == 1)
                {
                    IDisplay.clearscreen();
                    Serial.print("Reading basic data: length = ");
                    Serial.println(currentData.length());

                    const int speedOffset = 1;
                    const int instructionOffset = 2;
                    const int textOffset = 3;

                    if (currentData.length() > textOffset)
                    {
                        IDisplay.drawMessage(128, 40, 128, currentData.c_str() + textOffset, ArialMT_Plain_16, TEXT_ALIGN_RIGHT);
                    }

                    if (currentData.length() > instructionOffset)
                    {
                        DrawDirection(currentData.c_str()[instructionOffset]);
                    }

                    if (currentData.length() > speedOffset)
                    {
                        DrawSpeed(currentData.c_str()[speedOffset]);
                    }

                    IDisplay.displayscreen();
                }
                else
                {
                    Serial.println("invalid first byte");
                }
            }
        }
        else
        {
            uint32_t time = millis();
            if (time - g_lastActivityTime > 4000)
            {
                g_lastActivityTime = time;
                g_pCharIndicate->indicate();
            }
        }
    }
    else
    {
        g_lastTDeepsleep = millis();
        if(g_lastTDeepsleep - g_currentTDeepsleep > (50000)){
            Serial.println("Going to sleep now");
            IDisplay.clearscreen();
            IDisplay.drawMessage(64, 10, 128, "Sleeping!", ArialMT_Plain_16, TEXT_ALIGN_CENTER);
            IDisplay.displayscreen();
            delay(1000);
            IDisplay.clearscreen();
            IDisplay.displayOff();
            esp_deep_sleep_start();
        }
        else if(((g_lastTDeepsleep - g_currentTDeepsleep)/1000) != (g_displayDisSecond)){
            IDisplay.clearscreen();
            IDisplay.drawMessage(64, 10, 128, "Disconnected!", ArialMT_Plain_16, TEXT_ALIGN_CENTER);
            g_displayDisSecond = ((g_lastTDeepsleep - g_currentTDeepsleep)/1000);
            sprintf(g_displayDisSecond_Char, "Count: %u (s)", (unsigned int)g_displayDisSecond);
            IDisplay.drawMessage(64, 30, 128, g_displayDisSecond_Char, ArialMT_Plain_16, TEXT_ALIGN_CENTER);
            IDisplay.displayscreen();
        }
    }
    delay(10);
}

