/*
    *** Initialize LEM Device ***

    This code initializes the LEM device: it deletes all files on its memory and adjusts the time of the real-time clock to prepare the device to be deployed on the field.
    This code should be used to:
    1) Setup a newly built LEM device ;
    2) Reinitialize a LEM device after a completed experiment/deployment (the data from that experiment should have been saved correctly using the "Read LEM Device" code).

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
#include <vector>
#include "RTClib.h"

#define FORMAT_SPIFFS_IF_FAILED true

RTC_PCF8523 rtc;

void setup() {
  Serial.begin(115200);
  delay(5000);
  Serial.println(" ");
  Serial.println("*** Initialize LEM Device ***");
  Serial.println("Running this code will:");
  Serial.println("- reinitialize the LEM device and delete ALL files in memory;");
  Serial.println("- set the real-time clock (RTC) using the time of your computer.");
  Serial.println("-------------------------");

  setSPIFFS();
  listDir(SPIFFS, "/", 0);

  Serial.println("Are you sure you want to initialize the LEM device? (Y/N)");

  while (!Serial.available());

  char userInput = Serial.read();

  if (userInput == 'Y' || userInput == 'y') {
    Serial.println("-------------------------");
    setRTC();
    Serial.println("-------------------------");
    deleteAllFiles(SPIFFS, "/");
    listDir(SPIFFS, "/", 0);

    if (SPIFFSIsEmpty()) {
      Serial.println("LEM device memory is empty!");
    } 
    else {
      Serial.println("LEM device memory is not empty! Try reboot the LEM.");
    }
    getDateTime();
  }

  else if (userInput == 'N' || userInput == 'n') {
    Serial.println("LEM device has not been reinitialized.");
  }

  else {
    Serial.println("Your selection is not valid. Please reboot LEM device and try again.");
  }
}

void loop() {
}

void setSPIFFS() {
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("SPIFFS memory setup failed");
    return;
  } else {
    Serial.println("SPIFFS memory setup succeded.");
  }
}

void setRTC() {
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC.");
    Serial.println("Please verify there is a battery in the RTC chip.");
    Serial.flush();
    while (1) delay(10);
  }

  else {
    Serial.println("RTC setup succeded.");
  }

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  rtc.start();
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("List of files in the directory: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(" - not a directory");
    return;
  }

  std::vector<String> fileNames;

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      fileNames.push_back(String(file.name()) + "/");
    } else {
      fileNames.push_back(file.name());
    }

    file = root.openNextFile();
  }

  std::sort(fileNames.begin(), fileNames.end());

  for (const String &name : fileNames) {
    Serial.print("  ");
    if (levels > 0 && name.endsWith("/")) {
      Serial.print("DIR : ");
      Serial.println(name);
      listDir(fs, (dirname + name).c_str(), levels - 1);
    } else {
      Serial.print("FILE: ");
      Serial.print(name);
      File file = fs.open((dirname + name).c_str());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
      file.close();
    }
  }
}

void deleteAllFiles(fs::FS &fs, const String &dirPath) {
  Serial.printf("Deleting files in directory: %s\r\n", dirPath.c_str());

  File dir = fs.open(dirPath.c_str());
  if (!dir || !dir.isDirectory()) {
    Serial.println("- not a valid directory");
    return;
  }

  File file = dir.openNextFile();
  while (file) {
    String filePath = file.name();
    if (fs.remove(filePath.c_str())) {
      Serial.println("- deleted file: " + filePath);
    } else {
      Serial.println("- delete failed for file: " + filePath);
    }
    file = dir.openNextFile();
  }
}

bool SPIFFSIsEmpty() {
  File root = SPIFFS.open("/");
  if (!root) {
    Serial.println("Failed to open directory");
    return false;
  }

  bool isEmpty = true;
  File file = root.openNextFile();
  while (file) {
    isEmpty = false;
    file.close();
    file = root.openNextFile();
  }
  root.close();
  return isEmpty;
}

void getDateTime() {
  Serial.print("This is the date and time set on the RTC. Please check it is correct: ");
  DateTime now = rtc.now();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
}
