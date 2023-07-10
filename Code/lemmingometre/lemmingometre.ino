///// LIBRARIES /////
#include "RTClib.h"
#include <SPI.h>
#include <SD.h>

///// PIN AND SENSOR VARIABLES /////
File myFile;
char filename[50];
RTC_PCF8523 rtc;

int chipSelect = D8;  //SD card
int obstacleSensor = D4;

///// DATA VARIABLES /////
int obstacleValue;

///// MAIN CODE /////
void setup() {
  // Setup serial monitor and sensors
  Serial.begin(9600);
  Serial.println("Wemos awake");
  pinMode(obstacleSensor, INPUT);

  setupRTC();
  setupSD();


  // Data logging
  unsigned long startTime = millis();
  startLogFile();
  while (millis() - startTime < 1000) {
    readObstacleSensor();
    getTimeStamp();
  }

  endLogFile();
  readLogFile();
  Serial.println("Going to sleep");

  // Wemos goes to sleep
  ESP.deepSleep(0);
}

void loop() {
}

void readObstacleSensor() {
  obstacleValue = !digitalRead(obstacleSensor);
  Serial.print("Lemming count: ");
  myFile.print("Lemming count: ");
  Serial.println(obstacleValue);
  myFile.println(obstacleValue);
}

void setupRTC() {
#ifndef ESP8266
  while (!Serial)
    ;  // wait for serial port to connect. Needed for native USB
#endif
  if (!rtc.begin()) {
    Serial.println("RTC initialization failed");
    Serial.flush();
    while (1) delay(10);
  }
  rtc.start();
  Serial.println("RTC initialization done.");
}

void setupSD() {
  if (!SD.begin(chipSelect)) {
    Serial.println("SD initialization failed.");
    return;
  }
  Serial.println("SD card initialization done.");
}

void getTimeStamp() {
  rtc.start();
  DateTime now = rtc.now();

  myFile.print(now.year(), DEC);
  Serial.print(now.year(), DEC);
  myFile.print('/');
  Serial.print('/');
  myFile.print(now.month(), DEC);
  Serial.print(now.month(), DEC);
  myFile.print('/');
  Serial.print('/');
  myFile.print(now.day(), DEC);
  Serial.print(now.day(), DEC);
  myFile.print(" ");
  Serial.print(" ");
  myFile.print(now.hour(), DEC);
  Serial.print(now.hour(), DEC);
  myFile.print(':');
  Serial.print(':');
  myFile.print(now.minute(), DEC);
  Serial.print(now.minute(), DEC);
  myFile.print(':');
  Serial.print(':');
  myFile.print(now.second(), DEC);
  Serial.print(now.second(), DEC);
  myFile.println();
  Serial.println();
}

void startLogFile() {
  DateTime now = rtc.now();
  
  snprintf(filename, 50, "%d-%d-%d-%d-%d-%d.txt", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
  myFile = SD.open(filename, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    // Serial.print("Writing to test.txt...");
    // myFile.println("testing 1, 2, 4.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}

void readLogFile() {
  // re-open the file for reading:
  myFile = SD.open(filename);
  if (myFile) {
    Serial.println("saved data:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) { Serial.write(myFile.read()); }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening file");
  }
}
void endLogFile() {
  myFile.close();
}