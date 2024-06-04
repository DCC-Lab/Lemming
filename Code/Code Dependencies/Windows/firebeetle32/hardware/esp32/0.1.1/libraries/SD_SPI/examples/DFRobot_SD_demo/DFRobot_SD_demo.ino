/*
  SD card read/write

 This example shows how to read and write data to and from an SD card file
 The circuit:
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

 created   Nov 2010
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe

 This example code is in the public domain.

 */

#include <SPI.h>
#include <SD_SPI.h>

#ifdef __AVR__
uint8_t       pin_cs = 3;
#else
uint8_t       pin_cs = D3;
#endif

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println();
  Serial.println();
  if (!SD.begin()) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  Serial.print("    file name:");
  Serial.println(myFile.name());
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    //Serial.print("Writing to test.txt...");
    //myFile.println("testing 1, 2, 3.");
    // close the file:
    myFile.close();
    //Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  if(myFile) {
    Serial.println("file still have value");
  } else {
    Serial.println("file closed");
  }
  // re-open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.print("    file name:");
    Serial.println(myFile.name());
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
  
  Serial.print("    file name:");
  Serial.println(myFile.name());
  myFile = SD.open("Floder/nText.txt");
  if (myFile) {
    Serial.println("Floder/nText.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening Floder/nText.txt");
  }

  Serial.println("\ncreat direction");
  SD.mkdir("T1");

  setSDFileSystem(&myFile, &card, &volume, &root);
  Serial.println("read file:");
  uint8_t       buf[32] = {0};
  uint8_t       i = 0;
  readSDFile("test.txt", 32, buf, SD_FIRST | SD_CLOSE);
  for(i = 0; i < 32; i ++) {
    Serial.print(" ");
    Serial.print(buf[i], HEX);
  }
  Serial.println();
}

void loop() {
  // nothing happens after setup
}

