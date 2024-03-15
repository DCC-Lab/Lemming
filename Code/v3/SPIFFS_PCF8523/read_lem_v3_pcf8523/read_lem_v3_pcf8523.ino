/*
    *** Read LEM Device ***
    Description to be completed.
    
    This code should be used at the end of an experiment/deployment to get the data.
    
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
#include <vector>

#define FORMAT_SPIFFS_IF_FAILED true

void setup(){
  Serial.begin(115200);
  delay(5000);
  SPIFFS.begin(false);
  Serial.println(" ");
  Serial.println("*** Read LEM Device ***");
  listDir(SPIFFS, "/", 0); // Voir la liste des fichiers contenu dans le SPIFFS

}

void loop() {
  // if (Serial.available() > 0) {
  //   char incomingChar = Serial.read(); // Read the incoming character

  //   if (incomingChar == 'R') {
  //     // Send data to Python when 'R' is received
  //     listAndSendFiles("/");
  //   }
  // }
}


/*

// void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
//     Serial.printf("Listing directory: %s\r\n", dirname);

//     File root = fs.open(dirname);
//     if(!root){
//         Serial.println("- failed to open directory");
//         return;
//     }
//     if(!root.isDirectory()){
//         Serial.println(" - not a directory");
//         return;
//     }

//     File file = root.openNextFile();
//     while(file){
//         if(file.isDirectory()){
//             Serial.print("  DIR : ");
//             Serial.println(file.name());
//             if(levels){
//                 listDir(fs, file.name(), levels -1);
//             }
//         } else {
//             Serial.print("  FILE: ");
//             Serial.print(file.name());
//             Serial.print("\tSIZE: ");
//             Serial.println(file.size());
//         }
//         file = root.openNextFile();
//     }
// }

*/

void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
    Serial.printf("Listing directory: %s\r\n", dirname);

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

void listAndSendFiles(const char *dirname) {
    File root = SPIFFS.open(dirname);
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
            // Recursively list and send files in subdirectories
            listAndSendFiles(file.name());
        } else {
            // Read and send the content of the file
            Serial.print("Sending file: ");
            Serial.println(file.name());
            while (file.available()) {
                Serial.write(file.read());
            }
            Serial.println("End of file"); // Add a newline to indicate the end of the file
        }
        file = root.openNextFile();
    }
  Serial.println("End");
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return;
    }

    Serial.println("- read from file:");
    while(file.available()){
        Serial.write(file.read());
    }
}


