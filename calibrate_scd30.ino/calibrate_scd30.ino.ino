/*
  Reading CO2, humidity and temperature from the SCD30
  Displaying via 16x2 LED and servo driben gauge 
  By: Lars Seebach
  Date: 2020-11-09
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  
  Hardware Connections:
  Attach RedBoard to computer using a USB cable.
  Connect SCD30 LED display 16x2 to ESP8266 
  TODO: details on HW
  Open Serial Monitor at 115200 baud.

  Display at address 0x27
  SCD30 at address 0x61
  BME280 at address 0x76
  
*/

#include <cmath>
#include "SparkFun_SCD30_Arduino_Library.h" 
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Servo.h>
#include <Adafruit_BMP280.h>
#include <inttypes.h>


#define LCD_COLUMNS 16
#define LCD_ROWS 2

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, LCD_COLUMNS, LCD_ROWS);  

SCD30 airSensor;

Adafruit_BMP280 bmp; // I2C

boolean unkalibriert;
long laufzeit;

void setup() {
  delay(2000);
   Wire.begin();
   Wire.setClock(50000); //50kHz

  lcd.init();
  lcd.backlight();
  lcd.clear();

  if (airSensor.begin() == false)
  {
    while (1)
      ;
  }

  unkalibriert = true;
  lcd.setCursor(0, 0);
  lcd.print("bring outside!");
  delay(60000);

}

void loop() {

if (unkalibriert) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("kalibriere...");
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("...fertig");
  }
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print(airSensor.getCO2());
  delay(1000);
  laufzeit = millis();
  if (unkalibriert && laufzeit > 240000) {    //nach 4 min erfolgt Kalibrierung
    airSensor.setForcedRecalibrationFactor(450);
    unkalibriert = false;
  }

}
