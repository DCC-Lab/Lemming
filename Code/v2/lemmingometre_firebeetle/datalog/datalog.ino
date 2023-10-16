
#define BUTTON_PIN_BITMASK 0x200000000 // 2^33 in hex

int obstacleValue;
int pirValue;
int obstacle_acquisition_time = 3000;


void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void setup(){
  Serial.begin(57600);

  pinMode(GPIO_NUM_25, INPUT_PULLUP);
  pinMode(GPIO_NUM_26, INPUT); //Obstacle sensor
  pinMode(GPIO_NUM_27, INPUT); //DeepSleep
  pinMode(GPIO_NUM_2, INPUT_PULLDOWN);


  //Print the wakeup reason for ESP32
  print_wakeup_reason();


  unsigned long startTime = millis();

  while (millis() - startTime < obstacle_acquisition_time) {
    obstacleValue = readObstacleSensor();
    pirValue = readPIR1();
    printObstacleSensorReading();
    printPIR1();
    delay(200);
  }

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_27,RISING); //1 = High, 0 = Low


  //Go to sleep now
  Serial.println("Going to sleep now");

  esp_deep_sleep_start();
}

int readObstacleSensor() {
  return digitalRead(GPIO_NUM_26);
}

int readPIR1() {
  return digitalRead(GPIO_NUM_27);
}

void printObstacleSensorReading() {
  Serial.print("Lemming count: ");
  Serial.println(obstacleValue);
}

void printPIR1() {
  Serial.print("PIR state: ");
  Serial.println(pirValue);
}




void loop(){
  //This is not going to be called
}
