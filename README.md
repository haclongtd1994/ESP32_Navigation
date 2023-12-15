This project have refered from https://github.com/alexanderlavrushko/BLE-HUD-navigation-ESP32/tree/main.
Thank @alexanderlavrushko, I tried to find any license in your project but didn't seen. So, if have any violation, please direct contact me.
Modified Hardware support:
* ESP32 with external OLED SSD1306 display 128x64 (enabled by-default), [how to connect](https://github.com/alexanderlavrushko/BLE-HUD-navigation-ESP32#display-oled-128x64)

# Bluetooth Low Energy head-up display

https://www.tiktok.com/@codergacon/video/7162905984959106330

![Prototype](/images/IMG_BLE_HUD.png)

## How it works
The application on the phone sends instructions to ESP32 using Bluetooth Low Energy:
* The phone acts as BLE Central (also called Master, Client)
* ESP32 acts as BLE Peripheral (also called Slave, Server)

### Application iOS
Currently the only compatible application is available on iOS: [Sygic GPS Navigation & Maps](https://apps.apple.com/us/app/sygic-gps-navigation-maps/id585193266)

To enable BLE HUD in the app (it's an unofficial prototype feature):
* Menu / Settings / Info / About / tap 3 times on any item (new line About appears at the top) / About / BLE HUD / Start
* Bluetooth permission required

Notes:
* When BLE HUD enabled, the application automatically scans and connects to ESP32 module (the app must be in foreground)
* When already connected and the route is set, the phone screen can be turned off
* The feature becomes off after restarting the application
* During route recompute (e.g. when I miss my turn) "No route" message can temporarily appear on ESP32 

### Supported ESP32 modules
* ESP32 with external OLED display 128x128 (enabled by-default), [how to connect](https://github.com/alexanderlavrushko/BLE-HUD-navigation-ESP32#display-oled-128x128)
* ESP32 TTGO T-Display with embedded TFT 135x240, [how to enable](https://github.com/alexanderlavrushko/BLE-HUD-navigation-ESP32#ttgo-t-display)

### Technical info
1. Connection flow:
   1. Central (the application on the phone) scans for a Peripheral with wanted service UUID (see Table of UUIDs) - it's going to be our ESP32
   1. The application connects to the first found device
1. Sending instruction:
   1. Central writes new data to the characteristic (see Table of UUIDs) as soon as the data changes (current speed limit or an instruction)
   1. **Data example**: 0x01320A3335306D, meaning: basic data (0x01), current speed limit is 50km/h (0x32), turn right (0x0A) in 350m (0x3335306D, not null-terminated string)
   1. If the app doesn't send an update for a few seconds, ESP32 sends an empty indication (see Table of UUIDs), letting the app know that it wants a data update

### Table of UUIDs
Name | UUID
----- | ---------------
Service | DD3F0AD1-6239-4E1F-81F1-91F6C9F01D86
Characteristic for indicate | DD3F0AD2-6239-4E1F-81F1-91F6C9F01D86
Characteristic for data write | DD3F0AD3-6239-4E1F-81F1-91F6C9F01D86

### Display OLED SSD1306 128x64
Display: COEM 128x64 OLED IIC&SPI ([link](https://shopee.vn/B%E1%BA%A3ng-m%E1%BA%A1ch-m%C3%A0n-h%C3%ACnh-LCD-OLED-SSD1306-12864-0.96-inch-IIC-SPI-7-4-ch%E1%BA%A5u-nhi%E1%BB%81u-m%C3%A0u-s%E1%BA%AFc-cho-Arduino-i.148048328.16649282999?sp_atk=144d4113-5f2a-4d0f-880d-6ac3cff1494c&xptdk=144d4113-5f2a-4d0f-880d-6ac3cff1494c))

Protocol: SSD1306

Connected this way:
ESP32 | Display
----- | ---------------
GND | GND
3.3v | VCC
G33 | SCL
G27 | SDA
