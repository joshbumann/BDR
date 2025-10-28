#define USE_SERIAL1 1                 // 1: Serial1 (D19 RX1, D18 TX1). 0: SoftwareSerial on D10/D13
const bool ACTIVE_LOW = false;

#include <SoftwareSerial.h>

// RS-485 pins on UNO
const uint8_t RS485_RO_PIN = 4;  // RX from MAX485 RO
const uint8_t RS485_RE_PIN = 5;  // RE (LOW = listen, HIGH = talk)
const uint8_t RS485_DE_PIN = 6;  // DE (LOW = listen, HIGH = talk)
const uint8_t RS485_DI_PIN = 7;  // TX to MAX485 DI

SoftwareSerial RS485Serial(RS485_RO_PIN, RS485_DI_PIN); // rx, tx

inline void set485Listen() {          // RE=LOW, DE=LOW
  digitalWrite(RS485_RE_PIN, LOW);
  digitalWrite(RS485_DE_PIN, LOW);
}

void setup() {
  Serial.begin(115200);
  pinMode(RS485_RE_PIN, OUTPUT);
  pinMode(RS485_DE_PIN, OUTPUT);
  // Initialize RS485 communication (SoftwareSerial)
  set485Listen();              // idle in listen
  RS485Serial.begin(9600);     // must match MEGA
  //Serial.println(F("UNO RS485 Receiver ready."));

}

String incomingStr = "";

void loop() {
  set485Listen(); 
  if (RS485Serial.available()) {
    // Read incoming data from Mega
    set485Listen();
    String inStr = RS485Serial.readStringUntil('\n');
    inStr.trim();
    
    if(inStr.length() > 0){
      char incomingChar = inStr[0];
      //Serial.println(incomingChar);
      if(incomingChar != '>'){
        incomingStr += incomingChar;
      }
      else
      {
        // Forward the character to the laptop
        incomingStr += '>';
        Serial.println(incomingStr);
        incomingStr = ""; 
        Serial.flush();
        delay(50);
      }
     
    }
    //else{
      //Serial.println("No data incoming"); // Comment this out when transmitting to MATLAB / Debugging purposes only
    //}
  
  }

}
