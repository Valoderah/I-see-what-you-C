/*
 * Project: IoT Final Project: Badami's Board
 * Description: Can be written to from anywere with a text. Also displays time
 * and temp
 * Authors: Group 5: Hunter Walters, Valentine Osakwe, Pavithra Devdas
 * Date: 12/11/18
 */

/*       ---WIRING---
 * SCREEN             PHOTON
 * GND --------------- GND
 * VIN --------------- 3V3
 * CLK --------------- A3
 * MISO -------------- A4
 * MOSI -------------- A5
 * CS ---------------- D4
 * D/C --------------- D5
*/

// Code for setting up screen
#if defined(PARTICLE)
  #include <Adafruit_mfGFX.h>
  #include "Adafruit_ILI9341.h"

  // For the Adafruit shield, these are the default.
  #define TFT_DC D5
  #define TFT_CS D4

#else
  #include "SPI.h"
  #include <Adafruit_mfGFX.h>
  #include "Adafruit_ILI9341.h"

  // For the Adafruit shield, these are the default.
  #define TFT_DC 9
  #define TFT_CS 10
#endif

// DHT setup
#include "PietteTech_DHT.h"
#define DHTTYPE  DHT22       // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   D3          // Digital pin for communications

// Globals and initialization
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, 0);
PietteTech_DHT DHT(DHTPIN, DHTTYPE);

int button = D1;
int buzzer = D2;
int led = A1;
unsigned long trueTime = Time.now();
int timeChecker = 0;
int screenSaverOn = 0; //0 if off, 1 if on



// setup() runs once, when the device is first turned on.
void setup() {
  // Makes writing available to cloud
  Particle.function("writeMessage", writeMessage);

  Time.zone(-6);    // Sets timezone
  tft.begin();      // Start screen
  setScreenSaver();

  // Pin Modes
  pinMode(button, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  int result = DHT.acquireAndWait(1000); // wait up to 1 sec (default indefinitely)

  // Error handling for screen
  switch (result) {
  case DHTLIB_OK:
    Serial.println("OK");
    break;
  case DHTLIB_ERROR_CHECKSUM:
    Serial.println("Error\n\r\tChecksum error");
    break;
  case DHTLIB_ERROR_ISR_TIMEOUT:
    Serial.println("Error\n\r\tISR time out error");
    break;
  case DHTLIB_ERROR_RESPONSE_TIMEOUT:
    Serial.println("Error\n\r\tResponse time out error");
    break;
  case DHTLIB_ERROR_DATA_TIMEOUT:
    Serial.println("Error\n\r\tData time out error");
    break;
  case DHTLIB_ERROR_ACQUIRING:
    Serial.println("Error\n\r\tAcquiring");
    break;
  case DHTLIB_ERROR_DELTA:
    Serial.println("Error\n\r\tDelta time to small");
    break;
  case DHTLIB_ERROR_NOTSTARTED:
    Serial.println("Error\n\r\tNot started");
    break;
  default:
    Serial.println("Unknown error");
    break;
  }

  // If button pressed, re-write to screen
  if(digitalRead(button) == LOW){
    setScreenSaver();
  }

  // This code rewrites the screen semi-often so time and temp are kept up to date
  // Will only actually write if screen saver is currently up. No overwriting here :)
  timeChecker++;
  if(timeChecker >= 20000){
    timeChecker = 0;
    if(screenSaverOn == 1)
      setScreenSaver();
  }
}

// Called by IFTTT. Writes the texted message to the screen
int writeMessage(String message){
  screenSaverOn = 0;
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0,0);
  tft.setRotation(1);
  tft.setTextSize(4);
  tft.println(message);
  tone(buzzer, 1000, 500);
  flashLED(led);
  return 0;
}

/* When called, creates the default screen saver with time & temp. Also re-pings
   the true time so that time is kept accurate */
void setScreenSaver(){
  screenSaverOn = 1;      // screen saver is on

  // reseting screen
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0,0);
  tft.setRotation(1);
  tft.setTextSize(4);

  trueTime = Time.now();  // Repinging time

  //Prints a formats time
  tft.print(Time.hourFormat12(trueTime));
  tft.print(":");
  if(Time.minute(trueTime) < 10){
    tft.print("0");
  }
  tft.print(Time.minute(trueTime));

  // Prints PM or AM
  if(Time.isPM(trueTime) == 1)
    tft.println(" PM");
  else
    tft.println(" AM");

  // Prints humidity and temperature
  tft.println();
  tft.setTextSize(3);
  tft.println("Temperature: ");
  tft.print(DHT.getCelsius());
  tft.println(" C");
  tft.println("Humidity: ");
  tft.print(DHT.getHumidity());
  tft.println("%");
}

// Flashes a given LED
void flashLED(int led){
  digitalWrite(led, HIGH);
  delay(500);
  digitalWrite(led, LOW);
}
