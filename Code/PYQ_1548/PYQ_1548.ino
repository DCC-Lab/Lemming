const uint8_t tsl = 1; // Should be between 200 ns and 2000 ns so 1 us with microDelay()
const uint8_t tsh = 1; 
const uint8_t tshd = 72;
const uint8_t tslt = 580;
const uint8_t tdlink = 35;

const int dlink = D0;
const int serin = D4;

int obstacle_acquisition_time = 10000;
unsigned long startTime = millis();

                  // [.        tresh        ][blind time][ PC.][ WT ][ OM ][ SS ][ Res][C][R][P]
  int conf_reg[25] = {0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0}; //TRESH 100

void setup() {

  Serial.begin(9600);

  pinMode(serin, OUTPUT);
  send_serin();
  pinMode(dlink, OUTPUT);
  digitalWrite(dlink, LOW);
  delayMicroseconds(tdlink);
  pinMode(dlink, INPUT);
 
  
  // bool loop = true;
  unsigned long startTime = millis();
  // while (millis() - startTime < obstacle_acquisition_time) {
  // while(loop){
  //   if(digitalRead(dlink) == HIGH){
  //     Serial.println("Movement");
  //     pinMode(dlink, OUTPUT);
  //     digitalWrite(dlink, LOW);
  //     delayMicroseconds(tdlink);
  //     pinMode(dlink, INPUT);
  //     delay(10);
  //     Serial.println("Going to sleep");
  //     loop = false;
  //     // ESP.deepSleep(0);
  //   }
  // }

  // Serial.println("Going to sleep");
  // // ESP.deepSleep(0);

}

void loop() {
  if(millis() - startTime < obstacle_acquisition_time){
    if(digitalRead(dlink) == HIGH){
        Serial.println("Movement");
        pinMode(dlink, OUTPUT);
        digitalWrite(dlink, LOW);
        delayMicroseconds(tdlink);
        pinMode(dlink, INPUT);
        delay(10);
        // Serial.println("Going to sleep");
        // loop = false;
        // ESP.deepSleep(0);
    }
  }
  else{
    Serial.println("Going to sleep");
    ESP.deepSleep(0);
  }
}

void send_serin(){
  for(int i = 0; i<25; i++){
    digitalWrite(serin, LOW);
    delayMicroseconds(tsl);
    digitalWrite(serin, HIGH);
    delayMicroseconds(tsh);
    digitalWrite(serin, conf_reg[i]);
    delayMicroseconds(tshd);
  }
  digitalWrite(serin, LOW);
  delayMicroseconds(tslt);
}


