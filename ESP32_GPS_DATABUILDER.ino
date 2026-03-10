#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPS++.h>
#include <SPI.h>
#include <SD.h>

// --- Screen & GPS ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
TinyGPSPlus gps;

// --- SPI Remapping (Crucial to avoid D10 Boot-Hang) ---
#define SCK_PIN  8   // D8
#define MISO_PIN 9   // D9
#define MOSI_PIN 5   // D3 (GPIO 5)
#define CS_PIN   4   // D2

// --- Rotary Encoder Pins ---
#define ENCODER_CLK 20 // D7 (GPIO 20)
#define ENCODER_DT  21 // D6 (GPIO 21)
#define ENCODER_SW  10 // D10 (GPIO 10)

// --- State Variables ---
int menuPage = 0; 
int lastClk = HIGH;
bool sdActive = false;
bool isLogging = false;
unsigned long lastRefresh = 0;

void setup() {
  Serial.begin(115200);
  
  // 1. Start I2C (Screen)
  Wire.begin(6, 7);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED Fail"));
  }
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println(F("MONOLITH BOOT..."));
  display.display();
  delay(1000);

  // 2. Remap SPI and Start SD
  display.println(F("REMAPPING SPI..."));
  display.display();
  
  // Initialize custom SPI bus for SD Card
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
  
  if (!SD.begin(CS_PIN)) {
    Serial.println(F("SD Card Failed/Missing"));
    sdActive = false;
  } else {
    sdActive = true;
    File dataFile = SD.open("/gps_log.csv", FILE_APPEND);
    if (dataFile) { 
      dataFile.println("Time,Lat,Lng,Sats"); 
      dataFile.close(); 
    }
  }

  // 3. Setup Hardware Pins
  Serial1.begin(9600, SERIAL_8N1, 3, 2); // GPS on D1/D0
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
  pinMode(ENCODER_SW, INPUT_PULLUP);

  display.println(F("BOOT COMPLETE."));
  display.display();
  delay(1000);
}

void loop() {
  // 1. Handle GPS Data
  while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }

  // 2. Handle Rotary Encoder (Menu Selection)
  int newClk = digitalRead(ENCODER_CLK);
  if (newClk != lastClk && newClk == LOW) {
    if (digitalRead(ENCODER_DT) != newClk) menuPage++; else menuPage--;
    if (menuPage > 2) menuPage = 0;
    if (menuPage < 0) menuPage = 2;
  }
  lastClk = newClk;

  // 3. Handle Encoder Button (Toggle REC)
  if (digitalRead(ENCODER_SW) == LOW) {
    if (sdActive) isLogging = !isLogging;
    delay(300); // Debounce delay
  }

  // 4. Update Dashboard (0.5s Refresh)
  if (millis() - lastRefresh >= 500) {
    lastRefresh = millis();
    display.clearDisplay();
    
    // Draw Static Header
    display.setTextSize(1);
    display.setCursor(0,0);
    display.print(F("PAGE:")); display.print(menuPage + 1);
    display.setCursor(55, 0);
    display.print(isLogging ? F("REC") : F("STBY"));
    display.setCursor(100, 0);
    display.print(gps.satellites.value()); display.print(F("s"));
    display.drawLine(0, 12, 128, 12, SSD1306_WHITE);

    // Render Menu Pages
    switch (menuPage) {
      case 0: showGpsPage(); break;
      case 1: showSignalPage(); break;
      case 2: showSdPage(); break;
    }
    
    // Periodic SD Logging (Every 5 seconds if REC is on)
    static unsigned long lastLogTime = 0;
    if (isLogging && gps.location.isValid() && (millis() - lastLogTime > 5000)) {
       logData();
       lastLogTime = millis();
    }

    display.display();
  }
}

void showGpsPage() {
  if (gps.location.isValid()) {
    display.setCursor(0, 18); display.print(F("LAT:"));
    display.setTextSize(2); display.setCursor(0, 28); display.print(gps.location.lat(), 5);
    display.setTextSize(1); display.setCursor(0, 45); display.print(F("LNG:"));
    display.setTextSize(2); display.setCursor(0, 50); display.print(gps.location.lng(), 5);
  } else {
    display.setCursor(20, 35); display.print(F("SEARCHING SKY..."));
  }
}

void showSignalPage() {
  display.setCursor(0, 20); display.println(F("SAT SIGNAL (SATS)"));
  int barWidth = map(gps.satellites.value(), 0, 12, 0, 120);
  display.drawRect(4, 35, 122, 12, SSD1306_WHITE);
  display.fillRect(6, 37, (barWidth > 118 ? 118 : barWidth), 8, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 55); display.print(F("ALT: ")); display.print(gps.altitude.meters()); display.print(F("m"));
}

void showSdPage() {
  display.setCursor(0, 20);
  display.print(F("SD: ")); display.println(sdActive ? F("OK") : F("ERR"));
  display.setCursor(0, 35);
  display.print(F("LOG: ")); display.println(isLogging ? F("LOGGING") : F("OFF"));
  display.setCursor(0, 50);
  display.print(F("FILE: gps_log.csv"));
}

void logData() {
  File dataFile = SD.open("/gps_log.csv", FILE_APPEND);
  if (dataFile) {
    dataFile.print(gps.time.value()); dataFile.print(",");
    dataFile.print(gps.location.lat(), 6); dataFile.print(",");
    dataFile.print(gps.location.lng(), 6); dataFile.print(",");
    dataFile.println(gps.satellites.value());
    dataFile.close();
  }
}