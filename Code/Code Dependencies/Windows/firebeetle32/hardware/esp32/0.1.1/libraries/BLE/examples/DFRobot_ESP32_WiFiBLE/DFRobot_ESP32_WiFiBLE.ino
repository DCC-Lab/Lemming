/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleWrite.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    Add wifi by Xiaowu
    Note：this demo need to chose 8M Flash(3MB APP/1.5MB FAT)
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <WiFi.h>
#include <WiFiMulti.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        0xdfb0
#define CHARACTERISTIC_UUID 0xdfb1

const uint16_t port = 80;
const char * host = "192.168.2.100";
WiFiClient client;
BLEServer *pServer = NULL;
BLEService *pService = NULL;
BLECharacteristic *pCharacteristic = NULL;

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        Serial.println("*********");
        Serial.print("New value: ");
        if(client.connected()){
          client.print(value.data());
        }
        for (int i = 0; i < value.length(); i++)
          Serial.print(value[i]);

        Serial.println();
        Serial.println("*********");
      }
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("1- Download and install an BLE scanner app in your phone");
  Serial.println("2- Scan for BLE devices in the app");
  Serial.println("3- Connect to DFRobot_ESP32");
  Serial.println("4- Go to CUSTOM CHARACTERISTIC in CUSTOM SERVICE and write something");
  Serial.println("5- See the magic =)");

  BLEDevice::init("DFRobot_ESP32");
  pServer = BLEDevice::createServer();

  pService = pServer->createService((uint32_t)SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                                         (uint32_t)CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE|
                                         BLECharacteristic::PROPERTY_NOTIFY
                                       ); 

  pCharacteristic->setCallbacks(new MyCallbacks());

  pCharacteristic->setValue("Hello World");
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
  WiFi.begin("hitest", "12345678");
  Serial.println();
  Serial.println();
  Serial.print("Wait for WiFi... ");
  while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

    delay(500);
}

void loop() {
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, port)) {
      Serial.println("connection failed");
      Serial.println("wait 5 sec...");
      delay(5000);
      return;
  }
  while(1){
    String line = client.readStringUntil('\r');
    if(line.length()>0){
      Serial.print("receive data :");
      Serial.println(line);
      pCharacteristic->setValue(line.c_str());
      pCharacteristic->notify();
    }
  }
}
