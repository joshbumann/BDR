// MEGA RECEIVER + RELAY ACTUATOR — RE LOW=listen, HIGH=talk
#define USE_SERIAL1 1                 // 1: Serial1 (D19 RX1, D18 TX1). 0: SoftwareSerial on D10/D13
const bool ACTIVE_LOW = false;        // true if relay IN=LOW means ON


#include <SoftwareSerial.h>

// RS-485 communication
const uint8_t RS485_RO_PIN = 13;    // RX from MAX485 RO (fallback)
const uint8_t RS485_DI_PIN = 10;    // TX to MAX485 DI (fallback)
SoftwareSerial RS485Serial(RS485_RO_PIN, RS485_DI_PIN);

// RS485 Direction Control Pins
const uint8_t RS485_DE_PIN = 11;  // DE pin of RS485 module
const uint8_t RS485_RE_PIN = 12;  // RE pin of RS485

// Relays on D26..D33
const uint8_t RELAY_PINS[8] = {26,27,28,29,30,31,32,33};
bool relayState[8] = {0,0,0,0,0,0,0,0};

const int NUM_VALVES = 8;


const int motorvalve1pin = 26;  // Fuel N2
const int motorvalve2pin = 27;  // LOX N2
const int pneuvalve1pin = 28;   // MFV
const int pneuvalve2pin = 29;   // MOV
const int pneuvalve3pin = 30;   // Fuel Vent
const int pneuvalve4pin = 31;   // LOX Vent
const int pneuvalve5pin = 32;   // Fuel Purge
const int pneuvalve6pin = 33;   // LOX Purge

inline void set485Listen() {          // RE=LOW, DE=LOW
  digitalWrite(RS485_RE_PIN, LOW);
  digitalWrite(RS485_DE_PIN, LOW);
}
inline void set485Talk() {            // RE=HIGH, DE=HIGH
  digitalWrite(RS485_RE_PIN, HIGH);
  digitalWrite(RS485_DE_PIN, HIGH);
}

inline void setRelay(uint8_t idx, bool on) {
  relayState[idx] = on;
  uint8_t lvl = ACTIVE_LOW ? (on ? LOW : HIGH) : (on ? HIGH : LOW);
  digitalWrite(RELAY_PINS[idx], lvl);
}
inline void toggleRelay(uint8_t idx) { setRelay(idx, !relayState[idx]); }


// Turn a specific valve ON
void openValve(int valveIndex) {
  if (valveIndex >= 0 && valveIndex < NUM_VALVES+27) {
    digitalWrite(valveIndex, HIGH);
  }
}

// Turn a specific valve OFF
void closeValve(int valveIndex) {
  if (valveIndex >= 0 && valveIndex < NUM_VALVES+27) {
    digitalWrite(valveIndex, LOW);
  }
}



// All valves close
void closeAllValves() {
    digitalWrite(motorvalve1pin, LOW);
    digitalWrite(motorvalve2pin, LOW);
    digitalWrite(pneuvalve1pin, LOW);
    digitalWrite(pneuvalve2pin, LOW);
    digitalWrite(pneuvalve3pin, LOW);
    digitalWrite(pneuvalve4pin, LOW);
    digitalWrite(pneuvalve5pin, LOW);
    digitalWrite(pneuvalve6pin, LOW);
}


// Toggle valve from current state
void toggleValve(int valveIndex) {
  if (valveIndex >= 0 && valveIndex < NUM_VALVES+27) {
    int currentState = digitalRead(valveIndex);
    digitalWrite(valveIndex, !currentState);
  }
}


void setup() {
  // Initialize RS485 communication (SoftwareSerial)
  //Serial.begin(115200);

  pinMode(RS485_RE_PIN, OUTPUT);
  pinMode(RS485_DE_PIN, OUTPUT);
  set485Listen();                       // idle in listen

  RS485Serial.begin(9600);
  // Make sure all pnumatic valves start OFF
  for (uint8_t i = 0; i < 8; ++i) {
    pinMode(RELAY_PINS[i], OUTPUT);
    setRelay(i, false);
  }
  
}

void loop() {

    set485Listen();
  
    if (RS485Serial.available()) {
        set485Listen();
        String command = RS485Serial.readStringUntil('\n');
        command.trim();

        if (command.length() > 0) {

          char option = command[0];
          
          switch (option) {
          case 'A':
            toggleValve(motorvalve1pin);
            break;
          case 'B':
            toggleValve(motorvalve2pin);
            break;
          case 'C':
            toggleValve(pneuvalve1pin);
            break;
          case 'D':
            toggleValve(pneuvalve2pin);
            break;
          case 'E':
            toggleValve(pneuvalve3pin);
            break;
          case 'F':
            toggleValve(pneuvalve4pin);
            break;
          case 'G':
            toggleValve(pneuvalve5pin);
            break;
          case 'H':
            toggleValve(pneuvalve6pin);
            break;
          case 'I':
            toggleValve(pneuvalve1pin); // MFV
            toggleValve(pneuvalve2pin); // MOV
            break;
          case 'J':
            toggleValve(motorvalve1pin); // N2 valves
            toggleValve(motorvalve2pin);
            break;
          case 'K':
            toggleValve(pneuvalve3pin); // Vent valves
            toggleValve(pneuvalve4pin);
            break;
          case 'L':
            toggleValve(pneuvalve5pin); // Purge valves
            toggleValve(pneuvalve6pin);
            break;
          case 'M':
            closeAllValves();
            break;
          default:
            //Serial.println("Invalid input. Enter 1-8.");
          break;

          RS485Serial.flush();
          delay(2);
          set485Listen();
          }
      }
    }
}
