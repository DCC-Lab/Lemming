/*
    *** Main LEM Device ***

    This is the main code that will allow the LEM device to count the lemmings. 
    When you upload the code on the LEM device for the first time, it will create a "setupFile" on its memory and ask you to unplug the device. 
    Then, the LEM device is ready to be deployed on the field. 
    Each time the LEM device is powered off and then powered on again, it will create a startFile. 
    If the PIR sensor detects a lemming, it will create a new .txt file named with the timestamp of the detection. 

    This code should be used AFTER running the "Initialize LEM Device" code.
    
    The circuit:
    This code should be used with the LEM v3 circuit board with this hardware:
    - FIREBEETLE ESP32 (DFR0478)
    - PCF8523 RTC
    - PIR sensor

    Authors:
    Mireille Quémener
    Thomas Shooner
*/

#include "FS.h"
#include "SPIFFS.h"
#include <ESP32Time.h>
#include "RTClib.h"

#define BUTTON_PIN_BITMASK 0x200000000  // 2^33 in hex
#define FORMAT_SPIFFS_IF_FAILED true

char fileName[50];

RTC_PCF8523 rtc;

RTC_DATA_ATTR bool firstCount = true;
RTC_DATA_ATTR int obstacleAcquisitionTime = 20000;  // trigger pendant 10 sec

int obstacleValue;

void setup() {
  Serial.begin(115200);
  SPIFFS.begin(false);

  pinMode(GPIO_NUM_26, INPUT);  //ObstacleSensor

  Serial.println("-------------------------");
  setupRTC();
  Serial.println("-------------------------");

  // If LEM device is powered on for the first time
  if (firstCount) { 

    // If there is already a setupFile on the SPIFFS, create a startFile
    if (doesFileExistInDirectory(SPIFFS, "/", "/setupFile")) {
      get_create_startFile();
      firstCount = false;
    }

    // If there is no setupFile on the SPIFFS, create one.
    else {
      delay(5000);
      Serial.println(" ");
      Serial.println("*** Main LEM Device ***");
      Serial.println("This code is uploaded on the LEM device for the first time. Let's create a setupFile.");
      writeFile(SPIFFS, "/setupFile", "Lemming Enumeration Module started!");
      listDir(SPIFFS, "/", 0);

      if (doesFileExistInDirectory(SPIFFS, "/", "/setupFile")) {
        Serial.println("The setupFile has been created successfully!");
        Serial.println("Disconnect LEM device now. Do not upload the code again or reboot the LEM device.");
      } else {
        Serial.println("setupFile creation has failed.");
      }
      delay(60000);
    }
  }

  // If the LEM device has aleady been powered on before (or if there is already a setupFile AND a startFile)
  else {
    String fileName = get_create_FileName();
    unsigned long startTime = millis();
    while (millis() - startTime < obstacleAcquisitionTime) {
      obstacleValue = readObstacleSensor();
      Serial.println(obstacleValue);
      appendFile(SPIFFS, fileName, String(obstacleValue));
      appendFile(SPIFFS, fileName, "\r\n");
      delay(200);
    }
  }

  // Deep sleep until the PIR detects a new lemming
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_26, RISING);
  esp_deep_sleep_start();
}

void loop() {
}

void setupRTC() {
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC.");
    Serial.println("Please verify there is a battery in the RTC chip.");
    Serial.flush();
    while (1) delay(10);
  } else {
    Serial.println("RTC setup succeded.");
  }
  rtc.start();
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
    return false;  // Directory doesn't exist or isn't valid
  }

  File file = dir.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      file = dir.openNextFile();
      continue;
    }

    if (fileName.equals(file.name())) {
      return true;  // File with the same name exists in the directory
    }

    file = dir.openNextFile();
  }

  return false;  // File with the specified name doesn't exist in the directory
}

int countFilesInDirectory(fs::FS &fs, const String &dirPath) {
  int fileCount = 0;

  File dir = fs.open(dirPath.c_str());
  if (!dir || !dir.isDirectory()) {
    return -1;  // Directory doesn't exist or isn't valid
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
  return digitalRead(GPIO_NUM_26);
}

String get_create_FileName() {
  String directoryPath = "/";  // Replace with your directory path
  int numOfFiles = countFilesInDirectory(SPIFFS, directoryPath);
  DateTime now = rtc.now();
  snprintf(fileName, 50, "/%04d-%d%d%d%d%d%d", numOfFiles, now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
  createFile(SPIFFS, fileName);

  return String(fileName);
}

String get_create_startFile() {
  char fileName[30];  // Adjusted length for number of files and separators

  String directoryPath = "/";  // Replace with your directory path
  int numOfFiles = countFilesInDirectory(SPIFFS, directoryPath);

  sprintf(fileName, "/%04d-%s", numOfFiles, "startFile");

  createFile(SPIFFS, fileName);

  return String(fileName);
}
