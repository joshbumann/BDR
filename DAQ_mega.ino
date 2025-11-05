#include <SoftwareSerial.h>
#include <HX711.h>

// Declare load cell objects
HX711 loadcell1;
HX711 loadcell2;
HX711 loadcell3;


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

const int PT_pin7 = A6;

const int PT_pin6 = A5;
const int PT_pin5 = A4;
const int PT_pin4 = A3;
const int PT_pin3 = A2;
const int PT_pin2 = A1;
const int PT_pin1 = A0;

const int LC1_A = 35; // Needs two analog pins
const int LC1_B = 37;
const int LC2_A = 31;
const int LC2_B = 33;
const int LC3_A = 27;
const int LC3_B = 29;


// Calibration factors (you’ll set these after calibration)
float calFactor1 = 2900;
float calFactor2 = 6050;
float calFactor3 = 7300;

// initialize voltage values for speed
int PT_read7 = 0;
int PT_read6 = 0;
int PT_read5 = 0;
int PT_read4 = 0;
int PT_read3 = 0;
int PT_read2 = 0;
int PT_read1 = 0;
String dataString = "null";

void setup() {

  Serial.begin(115200);

  pinMode(RS485_RE_PIN, OUTPUT);
  pinMode(RS485_DE_PIN, OUTPUT);

  // Initialize RS485 communication (SoftwareSerial)
  set485Listen();              // idle in listen
  RS485Serial.begin(9600);

  pinMode(PT_pin7, INPUT);
  pinMode(PT_pin6, INPUT);
  pinMode(PT_pin5, INPUT);
  pinMode(PT_pin4, INPUT);
  pinMode(PT_pin3, INPUT);
  pinMode(PT_pin2, INPUT);
  pinMode(PT_pin1, INPUT);

  pinMode(LC1_A, INPUT);
  pinMode(LC1_B, INPUT);
  pinMode(LC2_A, INPUT);
  pinMode(LC2_B, INPUT);
  pinMode(LC3_A, INPUT);
  pinMode(LC3_B, INPUT);

  loadcell1.begin(LC1_A, LC1_B);
  loadcell2.begin(LC2_A, LC2_B);
  loadcell3.begin(LC3_A, LC3_B);

  loadcell1.set_scale(calFactor1); // Adjust this value after calibration
  loadcell1.tare();                // Reset the scale to zero
  loadcell2.set_scale(calFactor2);
  loadcell2.tare();
  loadcell3.set_scale(calFactor3);
  loadcell3.tare();

}




void loop() {

  String dataString = "<";

  PT_read7 = analogRead(PT_pin7);
  PT_read6 = analogRead(PT_pin6);
  PT_read5 = analogRead(PT_pin5);
  PT_read4 = analogRead(PT_pin4);
  PT_read3 = analogRead(PT_pin3);
  PT_read2 = analogRead(PT_pin2);
  PT_read1 = analogRead(PT_pin1);

  float Force1 = loadcell1.get_units(3);
  float Force2 = loadcell2.get_units(3);
  float Force3 = loadcell3.get_units(3);


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
  dataString += String(PT_read6, 4);
  dataString += ",";
  dataString += String(PT_read7, 4);
  dataString += ",";
  dataString += String(Force1, 4);
  dataString += ",";
  dataString += String(Force2, 4);
  dataString += ",";
  dataString += String(Force3, 4);
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
  delay(150);


}
