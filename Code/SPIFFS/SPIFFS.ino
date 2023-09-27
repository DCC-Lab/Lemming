#include "FS.h"
#include "SPIFFS.h"
#include <ESP32Time.h>

#define FORMAT_SPIFFS_IF_FAILED true

ESP32Time rtc;


void setup(){
  Serial.begin(115200);

  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
      Serial.println("SPIFFS Mount Failed");
      return;
  }






// ##### CE CODE PERMET DE VOIR LE NOM DES FICHIERS CRÉER DANS LE SPIFFS ####




  listDir(SPIFFS, "/", 0); // Voir la liste des fichiers contenu dans le SPIFFS

  // readFile(SPIFFS, "/exemple.txt"); // Ouvrir et lire un fichier contenu dans le SPIFFS avec le nom "/exemple.txt"

  // appendFile(SPIFFS, "/hello.txt", "World!\r\n"); // Ajouter du contenu au fichier "exemple.txt"

  // renameFile(SPIFFS, "/exemple.txt", "/exemple2.txt");  // Renommer le fichier "/exemple.txt" vers "exemple2.txt"
   
  // deleteFile(SPIFFS, "/exemple2.txt"); // Supprimer le fichier "exemple2.txt"

  // creatFile(SPIFFS, "hello.txt");  // Créer le fichier "hello.txt"



  // #### ATTENTION, CETTE FONCTION SUPPRIME L'ENTIÈRETÉ DES FICHERS CONTENUS DANS LE SPIFFS #####

  // deleteAllFiles(SPIFFS, "/"); // Supprimer tous les fichier du SPIFFS
    

}

void loop(){
}



void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
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

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("- file written");
    } else {
        Serial.println("- frite failed");
    }
}

void createFile(fs::FS &fs, const char * path){
  Serial.printf("Creating file: %s\r\n", path);
  File file = fs.open(path, FILE_WRITE);
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("- message appended");
    } else {
        Serial.println("- append failed");
    }
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\r\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("- file renamed");
    } else {
        Serial.println("- rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\r\n", path);
    if(fs.remove(path)){
        Serial.println("- file deleted");
    } else {
        Serial.println("- delete failed");
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

int countFilesWithBaseName(fs::FS &fs, const String &dirPath, const String &baseName) {
    int fileCount = 0;

    File dir = fs.open(dirPath.c_str());
    if (!dir || !dir.isDirectory()) {
        return fileCount; // Directory doesn't exist or isn't valid
    }

    File file = dir.openNextFile();
    while (file) {
        if (!file.isDirectory() && String(file.name()).startsWith("/" + baseName)) {
            fileCount++;
        }
        file = dir.openNextFile();
    }

    return fileCount;
}

String get_create_FileName() {
  char epochStr[20];
  char fileName[30]; // Adjusted length for number of files and separators
  itoa(rtc.getEpoch(), epochStr, 10);
  
  String directoryPath = "/"; // Replace with your directory path
  int numOfFiles = countFilesInDirectory(SPIFFS, directoryPath);
  
  sprintf(fileName, "/%04d-%s", numOfFiles, epochStr);
  
  createFile(SPIFFS, fileName);
  
  Serial.print("New file created at: ");
  Serial.println(rtc.getEpoch());
  
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
