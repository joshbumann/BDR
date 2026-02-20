#include <SoftwareSerial.h>



// RS-485 pins on UNO
// RS-485 communication
const uint8_t RS485_RO_PIN = 13;    // RX from MAX485 RO (fallback)
const uint8_t RS485_DI_PIN = 10;    // TX to MAX485 DI (fallback)
SoftwareSerial RS485Serial(RS485_RO_PIN, RS485_DI_PIN);

// RS485 Direction Control Pins
const uint8_t RS485_DE_PIN = 11;  // DE pin of RS485 module
const uint8_t RS485_RE_PIN = 12;  // RE pin of RS485

inline void set485Listen() {          // RE=LOW, DE=LOW
  digitalWrite(RS485_RE_PIN, LOW);
  digitalWrite(RS485_DE_PIN, LOW);
}
inline void set485Talk() {            // RE=HIGH, DE=HIGH
  digitalWrite(RS485_RE_PIN, HIGH);
  digitalWrite(RS485_DE_PIN, HIGH);
}


// Define analog pins for pressure sensors

//const int PT_pin7 = A6;
//const int PT_pin6 = A5;

const int PT_pin5 = A4;
const int PT_pin4 = A3;
const int PT_pin3 = A2;
const int PT_pin2 = A1;
const int PT_pin1 = A0;




// initialize voltage values for speed
//float PT_read7 = 0;
//float PT_read6 = 0;
float PT_read5 = 0;
float PT_read4 = 0;
float PT_read3 = 0;
float PT_read2 = 0;
float PT_read1 = 0;
String dataString = "null";

void setup() {

  Serial.begin(115200);

  pinMode(RS485_RE_PIN, OUTPUT);
  pinMode(RS485_DE_PIN, OUTPUT);

  // Initialize RS485 communication (SoftwareSerial)
  set485Listen();              // idle in listen
  RS485Serial.begin(9600);

  //pinMode(PT_pin7, INPUT);
  //pinMode(PT_pin6, INPUT);
  pinMode(PT_pin5, INPUT);
  pinMode(PT_pin4, INPUT);
  pinMode(PT_pin3, INPUT);
  pinMode(PT_pin2, INPUT);
  pinMode(PT_pin1, INPUT);
}




void loop() {

  String dataString = "<";

  
  PT_read5 = analogRead(PT_pin5);
  PT_read4 = analogRead(PT_pin4);
  PT_read3 = analogRead(PT_pin3);
  PT_read2 = analogRead(PT_pin2);
  PT_read1 = analogRead(PT_pin1);



  dataString += String(PT_read1, 4);
  dataString += ","; // csv
  dataString += String(PT_read2, 4);
  dataString += ",";
  dataString += String(PT_read3, 4);
  dataString += ",";
  dataString += String(PT_read4, 4);
  dataString += ",";
  dataString += String(PT_read5, 4);
  dataString += ",";
  //dataString += String(PT_read6, 4);
  //dataString += ",";
  //dataString += String(PT_read7, 4);
  //dataString += ",";

  dataString += ">"; // End delimiter
  dataString.trim();

  for (int i = 0; i < dataString.length(); ++i) {

    char cmd = dataString[i];

    if (cmd) {
      set485Talk();
      RS485Serial.write(cmd);
      RS485Serial.write('\n');
      RS485Serial.flush();   // ensure bytes left TX buffer
      delay(2);              // small guard at 9600 bps
      set485Listen();
    }
  }
  delay(50); // How low can we make this?
}