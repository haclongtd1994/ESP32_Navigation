#include <Arduino.h>
#include "main.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <Button2.h> // version 2.0.3, available in Arduino libraries, or https://github.com/LennartHennigs/Button2
#include "DataConstants.h"
#include "ImagesDirections.h"
#include "VoltageMeasurement.h"
#include "Display.h"

// -----------------
// Display selection
// Uncomment wanted display with corresponding header
// -----------------

// OLED 128x128 RGB, Waveshare 14747, driver SSD1351
// Doesn't require external libraries
// Pins: DIN=23, CLK=18, CS=5, DC=17, RST=16, uses SPIClass(VSPI)

SSD1306Wire oled(0x3c, 27, 33); //SDA pin 27, SCL pin 33
Display IDisplay(&oled);

// TTGO T-Display TFT 135x240
// Requires library TFT_eSPI from here: https://github.com/Xinyuan-LilyGO/TTGO-T-Display
// (copy TFT_eSPI to Arduino/libraries)
//#include "TFT_TTGO.h"
//TFT_TTGO selectedDisplay;
constexpr bool ENABLE_VOLTAGE_MEASUREMENT = false;

// ---------------------
// Variables for display
// ---------------------
#define DISPLAY_MIRRORED 0 // 0 or 1
#define DISPLAY_ROTATION 0 // 0, 90, 180 or 270

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

// --------
// Buttons
// --------
#define TTGO_LEFT_BUTTON 0
#define GPIO_NUM_TTGO_LEFT_BUTTON GPIO_NUM_0

#define TTGO_RIGHT_BUTTON 35
//#define GPIO_NUM_TTGO_RIGHT_BUTTON GPIO_NUM_35

#define BUTTON_DEEP_SLEEP TTGO_LEFT_BUTTON
#define GPIO_NUM_WAKEUP GPIO_NUM_TTGO_LEFT_BUTTON

Button2 g_btnDeepSleep(BUTTON_DEEP_SLEEP);
bool g_sleepRequested = false;

// --------
// Voltage measurement
// --------
#define VOLTAGE_ADC_ENABLE          14
#define VOLTAGE_ADC_PIN             34
static VoltageMeasurement g_voltage(VOLTAGE_ADC_PIN, VOLTAGE_ADC_ENABLE);
static bool g_showVoltage = false;
Button2 g_btn1(TTGO_RIGHT_BUTTON);

// ---------------------
// Bluetooth event callbacks
// ---------------------
class MyServerCallbacks: public BLEServerCallbacks
{
    void onConnect(BLEServer* pServer) override
    {
        g_deviceConnected = true;
        g_lastActivityTime = millis();
    }

    void onDisconnect(BLEServer* pServer) override
    {
        g_deviceConnected = false;
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

void setup()
{
    Serial.begin(115200);
    Serial.println("BLENaviPeripheral2 setup() started");

	IDisplay.init();
	IDisplay.drawWelcomeScreen();
    Serial.println("Display init done");

    // init BLE
    Serial.println("BLE init started");

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

    // setup deep sleep button
    g_btnDeepSleep.setLongClickTime(500);
    g_btnDeepSleep.setLongClickDetectedHandler([](Button2& b) {
        g_sleepRequested = true;
    });
    g_btnDeepSleep.setReleasedHandler([](Button2& b) {
        if (!g_sleepRequested)
        {
            return;
        }

        // IDisplay.displayOff();

        // without this the module won't wake up with button if powered from battery,
        // especially if entered deep sleep when powered from USB
        g_voltage.end();

        esp_sleep_enable_ext0_wakeup(GPIO_NUM_WAKEUP, 0);
        delay(200);
        esp_deep_sleep_start();
    });

    // setup voltage measurement
    if (ENABLE_VOLTAGE_MEASUREMENT)
    {
        g_voltage.begin();
        g_btn1.setPressedHandler([](Button2& b) {
            g_showVoltage = true;
        });
        g_btn1.setReleasedHandler([](Button2& b) {
            g_showVoltage = false;
        });
    }

    Serial.println("setup() finished");
}

void loop()
{
    g_btnDeepSleep.loop();
    g_btn1.loop();
    if (g_sleepRequested)
    {
        // Implement later show Sleep state ECU
    }
    else if (g_showVoltage)
    {
        static uint64_t voltageTimeStamp = 0;
        if (millis() - voltageTimeStamp > 1000)
        {
            voltageTimeStamp = millis();
            String voltageStr = String(g_voltage.measureVolts()) + " V";
            // Implement later show voltage
        }
    }
    else if (g_deviceConnected)
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
    else if (millis() > 3000)
    {

    }
    delay(10);
}


void DrawDirection(uint8_t direction)
{
    const uint8_t* imageProgmem = ImageFromDirection(direction);
    if (imageProgmem)
    {
        IDisplay.drawDirection(0, 0, 64, 64, imageProgmem);
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
