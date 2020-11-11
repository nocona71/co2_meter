/*
  Reading CO2, humidity and temperature from the SCD30
  Displaying via 16x2 LED and servo driben gauge 
  By: Lars Seebach
  SparkFun Electronics
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

#define LCD_COLUMNS 16
#define LCD_ROWS 2


SCD30 airSensor;

Adafruit_BMP280 bmp; // I2C

// set the LCD number of columns and rows

  

int lcdColumns = 16;
int lcdRows = 2;
float act_Humidity;
float act_Temperature;
uint16_t act_CO2;
float act_temp;
float act_alt;
float calc_alt;
int act_pressure = 1010; //default 1010 hPa

char co2_status[16] = "undefined";
int int_pos = 0;
int pos_new = 180;
int pos_old = 0;


// define German characters

byte ue_small[8]={
  B10001,
  B00000,
  B10001,
  B10001,
  B10001,
  B10011,
  B01101,};

byte ue_large[8]={
  B10001,
  B00000,
  B10001,
  B10001,
  B10001,
  B10001,
  B01110,};

Servo myservo;  // create servo object to control a servo

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, LCD_COLUMNS, LCD_ROWS);  

void setup()
{
  Serial.begin(115200);
  Serial.println("CO2 Meter");
  Wire.begin();   
  // reduce the bus speed to avoid weird charactes in the display
  Wire.setClock(10000);



  // set correct i2c address for BMP280

 if (!bmp.begin(0x76)) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (true);
  }


  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  lcd.createChar(1 , ue_small); 
  lcd.createChar(2 , ue_large); 

  if (airSensor.begin() == false)
  {
    Serial.println("Air sensor not detected. Please check wiring. Freezing...");
    while (1)
      ;
  }

  airSensor.setAltitudeCompensation(3); //Set altitude of the sensor in m
  // lt. https://de.wikipedia.org/wiki/Elmshorn Höhe: 3 m ü. NHN

  airSensor.setAmbientPressure(1010); //Current ambient pressure in mBar: 700 to 1200
  // grob geschätzt

  if (airSensor.begin() == false)
  {
//    Serial.println("Air sensor not detected. Please check wiring. Freezing...");
    while (1)
      ;
  }

  //The SCD30 has data ready every two seconds

  // initialize LCD
  lcd.init();
  lcd.backlight();
  
  // render the splash screen
  splash_screen();
                    
}

// Render a splash screen
void splash_screen()
{

  // set cursor to first column, first row
  lcd.setCursor(0, 0);
  lcd.print("Frischluft");
  lcd.setCursor(0, 1);
  lcd.print("f");
  lcd.write(1); 
  lcd.print("r alle");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Entwickelt von");
  lcd.setCursor(0, 1);
  lcd.print("Familie Seebach");
  delay(2000);
   lcd.clear();
}

void loop()
{

    act_temp = bmp.readTemperature();
    act_pressure = (int)round(bmp.readPressure()/100);
    act_alt = bmp.readAltitude(1013.25); /* Adjusted to local forecast! */

    calc_alt = 44330 * (1.0 - pow((act_pressure) / 1013.25, 0.1903));


    Serial.println("### BMP 280 measurements:");
    Serial.print("Temperature = ");
    Serial.print(act_temp);
    Serial.println(" *C");

    Serial.print("Pressure = ");
    Serial.print(act_pressure);
    Serial.println(" hPa");

    Serial.print("Approx altitude = ");
    Serial.print(act_alt); 
    Serial.println(" m");

    Serial.print("Calc altitude = ");
    Serial.print(calc_alt); 
    Serial.println(" m");

  // set the actual pressure measure in BP280 in SCD30
  airSensor.setAmbientPressure(act_pressure); 
  
  // check if data is available
  if (airSensor.dataAvailable())
  {

    if ( airSensor.getCO2() !=0 ){
      act_CO2 = airSensor.getCO2();
    }
    act_Temperature = airSensor.getTemperature();
    act_Humidity = airSensor.getHumidity();

    // use different messages for different CO2 levels
    if (act_CO2 <600) {
      strcpy (co2_status, "[ CO2 Level OK ]");
    }
    else if ((act_CO2 > 600) and (act_CO2 < 950)){
      strcpy (co2_status, "[ Bald luften  ]");
      //fix the German Umlaut
      co2_status[8]=1;
    }

    else if (act_CO2 > 950) {
      strcpy(co2_status, "[ JETZT LUFTEN ]");
      //fix the German Umlaut
      co2_status[9]=2;
    }


    Serial.println("### SDC 30 measurements:");

    Serial.print("co2(ppm): ");
    Serial.println(act_CO2);

    Serial.print("temp(C): ");
    Serial.println (act_Temperature, 1);

    Serial.print("humidity(%): ");
    Serial.println(act_Humidity, 1);
    
    Serial.print(co2_status);
    Serial.print(act_Humidity, 1);

    Serial.println();


  }

  else {

    // display via LCD
    update_display();

    //display via gauge
    update_gauge();

    delay(1000);

  }
//    act_temp = bmp.readTemperature();
//    act_pressure = bmp.readPressure()/100;
//    act_alt = bmp.readAltitude(1013.25); /* Adjusted to local forecast! */
//    ;
//


}


void update_display(){

    lcd.clear();
    lcd.setCursor(0, 0);
    // print CO2 level
    lcd.print((String)act_CO2);
    lcd.print("ppm ");
    // print temperature
    lcd.print((String)(int)round(act_Temperature));
    //print a degree symbol
    lcd.print((char)223);
    lcd.print("C ");
    // print humidity
    lcd.print((String)(int)round(act_Humidity));
    lcd.print("%");

    // print message
    lcd.setCursor(0, 1);
    lcd.print(co2_status);

    

    delay(1000);

    lcd.clear();
    lcd.setCursor(0, 0);
    // print pressure
    lcd.print((String)act_pressure);
    lcd.print("hPa ");
    // print Temp
    lcd.print((String)(int)round(act_temp));
    //print a degree symbol
    lcd.print((char)223);
    lcd.print("C ");
    // print altitude
    //lcd.print((String)(int)round(act_alt));
    //lcd.print("%");
        // print message
    lcd.setCursor(0, 1);
    lcd.print(co2_status);
  
}
  
void update_gauge(){

    // set servo position on gauge

    //   
    int_pos = (int)(round(act_CO2/10/5)*5);

    Serial.print("int_pos:");
    Serial.print(int_pos);

    // avoid exceeding the scale
    if (int_pos > 180) {
      int_pos = 180;
    }

    pos_new = 180-int_pos;

    Serial.print("pos_new:");
    Serial.println(pos_new);
 
    Serial.print("pos_old:");
    Serial.println(pos_old);

    // change posiition only if position changed significantly
    if (pos_new != pos_old) 
    {
      myservo.attach(13);  // pin 13 / D7
      myservo.write(pos_new); 
      Serial.print("moved servo moved to: ");
      Serial.println(pos_new);
      delay(1000);
      myservo.detach();
      pos_old = pos_new;      
    }


}
