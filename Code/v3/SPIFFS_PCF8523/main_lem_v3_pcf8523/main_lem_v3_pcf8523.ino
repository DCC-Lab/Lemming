#include "FS.h"
#include "SPIFFS.h"
#include <ESP32Time.h>
#include "RTClib.h"

// #define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
// #define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (in seconds) */
#define BUTTON_PIN_BITMASK 0x200000000 // 2^33 in hex
#define FORMAT_SPIFFS_IF_FAILED true

char fileName[50];

RTC_PCF8523 rtc;

RTC_DATA_ATTR bool firstCount = true;
RTC_DATA_ATTR int obstacle_acquisition_time = 20000; // trigger pendant 10 sec

int obstacleValue;
//uint16_t getTimeBuff[7];

void setup(){
  Serial.begin(115200);
  SPIFFS.begin(false);

  pinMode(GPIO_NUM_26, OUTPUT); // RTC transistor
  pinMode(GPIO_NUM_27, INPUT); //ObstacleSensor
  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
  rtc.start();

  if (firstCount){

    if(doesFileExistInDirectory(SPIFFS, "/", "/setupFile")){
      get_create_startFile();
      firstCount = false;
    }

    else{
      writeFile(SPIFFS, "/setupFile", "Lemming Enumeration Module started!");
      listDir(SPIFFS, "/", 0);
      Serial.println("Disconnect LEM device now.");
      delay(60000);
    }

  }

  else{

    String fileName = get_create_FileName();

    unsigned long startTime = millis();
    while (millis() - startTime < obstacle_acquisition_time) {
    obstacleValue = readObstacleSensor();
    Serial.println(obstacleValue);
    appendFile(SPIFFS, fileName, String(obstacleValue));
    appendFile(SPIFFS, fileName, "\r\n");

    delay(200);
    }
  }

    esp_sleep_enable_ext0_wakeup(GPIO_NUM_27,RISING);
    esp_deep_sleep_start();
    
}

void loop(){
}

void listDir(fs::FS &fs, const String &dirname, uint8_t levels) {
    Serial.printf("Listing directory: %s\r\n", dirname.c_str());

    File root = fs.open(dirname.c_str());
    if (!root) {
        Serial.println("- failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels) {
                listDir(fs, file.name(), levels - 1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}


void writeFile(fs::FS &fs, const String &path, const String &message) {
    // Serial.printf("Writing file: %s\r\n", path.c_str());

    File file = fs.open(path.c_str(), FILE_WRITE);
    if (!file) {
        // Serial.println("- failed to open file for writing");
        return;
    }
    if (file.print(message)) {
        // Serial.println("- file written");
    } else {
        // Serial.println("- write failed");
    }
}

void createFile(fs::FS &fs, const String &path) {
    // Serial.printf("Creating file: %s\r\n", path.c_str());
    File file = fs.open(path.c_str(), FILE_WRITE);
}

void appendFile(fs::FS &fs, const String &path, const String &message) {
    // Serial.printf("Appending to file: %s\r\n", path.c_str());

    File file = fs.open(path.c_str(), FILE_APPEND);
    if (!file) {
        // Serial.println("- failed to open file for appending");
        return;
    }
    if (file.print(message)) {
        // Serial.println("- message appended");
    } else {
        // Serial.println("- append failed");
    }
}


bool doesFileExistInDirectory(fs::FS &fs, const String &dirPath, const String &fileName) {
    File dir = fs.open(dirPath.c_str());
    if (!dir || !dir.isDirectory()) {
        return false; // Directory doesn't exist or isn't valid
    }

    File file = dir.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            file = dir.openNextFile();
            continue;
        }
        
        if (fileName.equals(file.name())) {
            return true; // File with the same name exists in the directory
        }
        
        file = dir.openNextFile();
    }

    return false; // File with the specified name doesn't exist in the directory
}

int countFilesInDirectory(fs::FS &fs, const String &dirPath) {
    int fileCount = 0;

    File dir = fs.open(dirPath.c_str());
    if (!dir || !dir.isDirectory()) {
        return -1; // Directory doesn't exist or isn't valid
    }

    File file = dir.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            fileCount++;
        }
        file = dir.openNextFile();
    }

    return fileCount;
}

int readObstacleSensor() {
  return digitalRead(GPIO_NUM_27);
}

void printObstacleSensorReading() {
  Serial.print("Lemming count: ");
  Serial.println(obstacleValue);
}

String get_create_FileName() {
  String directoryPath = "/"; // Replace with your directory path
  int numOfFiles = countFilesInDirectory(SPIFFS, directoryPath);
  DateTime now = rtc.now();
  snprintf(fileName, 50, "/%04d-%d%d%d%d%d%d", numOfFiles, now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
  createFile(SPIFFS, fileName);
  
  return String(fileName);
}

String get_create_startFile(){
  char fileName[30]; // Adjusted length for number of files and separators
  
  String directoryPath = "/"; // Replace with your directory path
  int numOfFiles = countFilesInDirectory(SPIFFS, directoryPath);
  
  sprintf(fileName, "/%04d-%s", numOfFiles, "startFile");
  
  createFile(SPIFFS, fileName);
  
  return String(fileName);
}


// 202310121123520
// 202310121123552





