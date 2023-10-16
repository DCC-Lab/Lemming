///// LIBRARIES /////
#include "RTClib.h"
#include <SPI.h>
#include <SD.h>

///// PIN AND SENSOR VARIABLES /////
File myFile;
char filename[50];
RTC_PCF8523 rtc;

int chipSelect = D8;  //SD card
int obstacleSensor = D0;

///// DATA VARIABLES /////
int obstacle_acquisition_time = 20000;
int obstacleValue;
DateTime now;

///// MAIN CODE /////
void setup() {
  // Setup serial monitor and sensors
  Serial.begin(9600);
  Serial.println("Wemos awake");
  pinMode(obstacleSensor, INPUT);

  setupRTC();
  setupSD();
  setFileName();
  writeFileHeading();

  // Data logging
  unsigned long startTime = millis();

  while (millis() - startTime < obstacle_acquisition_time) {
    now = rtc.now();
    printTimeStamp();

    obstacleValue = readObstacleSensor();
    printObstacleSensorReading();

    writeTimeAndCountInFile();
  }

  Serial.println("Going to sleep");
  // Wemos goes to sleep
  ESP.deepSleep(0);
}

void loop() {
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

void setFileName() {
  DateTime now = rtc.now();
  snprintf(filename, 50, "%d-%d-%d-%d-%d-%d.csv", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
}


void writeFileHeading() {
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open(filename, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing heading ...");
    myFile.print("Time stamp");
    myFile.print(",");
    myFile.println("Lemming count");
    myFile.close();
    Serial.println("done.");
  } else {
  // if the file didn't open, print an error:
    Serial.println("error opening file");
  }
}

void printTimeStamp(){
  //myFile.print(now.year(), DEC);
  Serial.print(now.year(), DEC);
  //myFile.print('/');
  Serial.print('/');
  //myFile.print(now.month(), DEC);
  Serial.print(now.month(), DEC);
  //myFile.print('/');
  Serial.print('/');
 // myFile.print(now.day(), DEC);
  Serial.print(now.day(), DEC);
 // myFile.print(" ");
  Serial.print(" ");
 // myFile.print(now.hour(), DEC);
  Serial.print(now.hour(), DEC);
 // myFile.print(':');
  Serial.print(':');
  //myFile.print(now.minute(), DEC);
  Serial.print(now.minute(), DEC);
 // myFile.print(':');
  Serial.print(':');
 // myFile.print(now.second(), DEC);
  Serial.print(now.second(), DEC);
  //myFile.println();
  Serial.println();
}

int readObstacleSensor() {
  return digitalRead(obstacleSensor);
}

void printObstacleSensorReading() {
  Serial.print("Lemming count: ");
  Serial.println(obstacleValue);
}



void writeTimeAndCountInFile() {
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
    }
  myFile = SD.open(filename, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing time stamp and count to file ...");
    myFile.print(now.year(), DEC);
    myFile.print('/');
    myFile.print(now.month(), DEC);
    myFile.print('/');
    myFile.print(now.day(), DEC);
    myFile.print(" ");
    myFile.print(now.hour(), DEC);
    myFile.print(':');
    myFile.print(now.minute(), DEC);
    myFile.print(':');
    myFile.print(now.second(), DEC);
    myFile.print(",");
    myFile.println(obstacleValue);

    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening file");
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



