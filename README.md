# Monolith Navigator: GPS Telemetry & Logging

![Monolith Navigator Demo](demo.jpg)

A basic GPS data logger built on the ESP32-C3 Super Mini. This project features a multi-page OLED menu system, real-time satellite SNR monitoring, and FAT32 SD card logging.

## Partslist
* **MCU:** ESP32-C3 Super Mini
* **GPS:** NEO-6M Module + Active Ceramic Antenna
* **Display:** 0.96" Dual-Color OLED (SSD1306)
* **Storage:** MicroSD SPI Module
* **Control:** KY-040 Rotary Encoder

A critical challenge in this build was the **GPIO 10 (D10) Conflict**. On the XIAO ESP32-C3, D10 is internally tied to the Flash memory. Initializing SPI on this pin causes a boot-loop. Solved by utilizing ESP32-C3's internal GPIO matrix to remap the MOSI line to D3, moving the mechanical Encoder Switch to D10. This ensures the SPI bus remains silent during the boot-up phase.

* `/ESP32_GPS_DATABUILDER.ino`: Main firmware.

## Pinouts
| Component | Pin Label | ESP32-C3 Pin | GPIO | Function |
| :--- | :--- | :--- | :--- | :--- |
| **GPS Module** | TX | **D0** | 2 | UART Data Out |
| **GPS Module** | RX | **D1** | 3 | UART Data In |
| **SD Card** | CS | **D2** | 4 | SPI Chip Select |
| **SD Card** | MOSI | **D3** | 5 | **Remapped SPI Data** |
| **OLED Screen** | SDA | **D4** | 6 | I2C Data |
| **OLED Screen** | SCL | **D5** | 7 | I2C Clock |
| **Encoder** | DT | **D6** | 21 | Rotation Data |
| **Encoder** | CLK | **D7** | 20 | Rotation Clock |
| **SD Card** | SCK | **D8** | 8 | SPI Clock |
| **SD Card** | MISO | **D9** | 9 | SPI Data Out |
| **Encoder SW** | SW | **D10** | 10 | **Menu Button** |

