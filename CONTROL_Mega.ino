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
bool sequenceTriggered[5] = {0,0,0,0,0}; // Track which sequences have been triggered (1-5)

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


// Turn a specific valve ON (FIX LATER: This turns the relay on. openValve on a vent will close it)
void openValve(int valveIndex) {
  
  if(valveIndex == 30 || valveIndex == 31) // Meant to handle the vents being normaly open
  {
    digitalWrite(valveIndex, LOW);
  }
  else if(valveIndex >= 26 && valveIndex <= 33) {
    digitalWrite(valveIndex, HIGH);
  }
}

// Turn a specific valve OFF (FIX LATER: This turns the relay off. closeValve on a vent will open it)
void closeValve(int valveIndex) {
  if(valveIndex == 30 || valveIndex == 31) // Meant to handle the vents being normaly open
  {
    digitalWrite(valveIndex, HIGH);
  }
  else if(valveIndex >= 26 && valveIndex <= 33) {
    digitalWrite(valveIndex, LOW);
  }
}

// All valves close
void closeAllValves() {
    digitalWrite(motorvalve1pin, LOW);
    digitalWrite(motorvalve2pin, LOW);
    digitalWrite(pneuvalve1pin, LOW);
    digitalWrite(pneuvalve2pin, LOW);
    digitalWrite(pneuvalve3pin, HIGH); // Vents must be high to be closed
    digitalWrite(pneuvalve4pin, HIGH); //
    digitalWrite(pneuvalve5pin, LOW);
    digitalWrite(pneuvalve6pin, LOW);
}


// Toggle valve from current state
void toggleValve(int valveIndex) {
  if (valveIndex >= 26 && valveIndex <= 33) {
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



// HOTFIRE VARIABLES

int currentHFstate = 0; // Keeps track of current state (0-5). 0 means coldflow mode.
bool engineFired = 0;   // Helps with logic for ending hot fire. Changes if engine has been fired.
int igniterPin = 0;

int delay_closevent_openMBVs = 0; // HFS2 : the delay betewen closing the vents and opening the MBVs. Something small just to act as a buffer not to vent any unessesary N2
int delay_openMBVs = 0;           // This is the standard time it takes for the MBVs to fully open. Having this delay in the code will safeguard against trying to do anything unless they are fully toggled.
int delay_closeMBVs = 0;          // Same as above, but in case closing takes a different time.
int delay_toggleVents = 0;        // This delay will go between opening and closing the vents. Just needs to be the amount of time to depressurize.

int delay_MFV_MOV = 0;            // This is the delay between opening the main valves. We want the liquids to enter the injector at the same time, and this takes into account that the fuel needs to travel through the regen channels.
int delay_igniter = 0;            // Delay between igniter firing and the main propellant valves opening.

int delay_closeMVs_openPurge = 0;       // Delay between closing the main valves and opening the the purge and vent lines
int delay_purge2vents = 0;              // Delay between closing MBVs and opening purge to closing vents
int delay_lengthPurge = 0;              // How long we want to the purge to run
int delay_closePurge_closeVents = 0;    // Short delay between closing the purge and vents so everything the the purge lines can escape


// HOTFIRE FUNCTIONS

void HF0toHF1(){
  // Default pos
  closeValve(26);
  closeValve(27);
  closeValve(28);
  closeValve(29);
  openValve(30); // Vents
  openValve(31);
  closeValve(32);
  closeValve(33);
}
void HF1toHF2(){
  // Close vents
  closeValve(30);
  closeValve(31);

  // Small delay to ensure vent close
  delay(delay_closevent_openMBVs);

  // Open MBVs
  openValve(26);
  openValve(27);
  delay(delay_openMBVs);

}
void HF2toHF3(){ // executes after Y command
  // Fire igniter
  pinMode(igniterPin, OUTPUT);
  digitalWrite(igniterPin, HIGH)

  // Delay for magnesium to catch
  delay(delay_igniter);

  // Open main prop valves, w/ delay for regen channels
  openValve(28);
  delay(delay_MFV_MOV);
  openValve(29);

}
void HF3toHF4(){
  // Close MPVs (This phase might be changed for backflow issues)
  closeValve(28);
  closeValve(29);

  delay(delay_closeMVs_openPurge);
  // Open purge, close MBVs
  openValve(32);
  openValve(33);

  closeValve(26);
  closeValve(27);

  delay(delay_purge2vents);
  // Open vents
  openValve(30);
  openValve(31);

}
void HF4toHF5(){
  // Close purge
  closeValve(32);
  closeValve(33);
  // End of test. Returns to coldflow state
}
void HF1toHF0(){
  // Does nothing
  // Leave in code for now incase we decide to do something with it
}
void HF2toHF0(){
  // Emergency abort: Close MBVs, open purge, and open vent simultaneously
  // Close MBVs
  closeValve(26);
  closeValve(27);

  // Open vents
  openValve(30);
  openValve(31);

  // Open purge
  openValve(32);
  openValve(33);
}
void HF2toHF1(){
  // Close MBVs, wait for full close, then open vents
  closeValve(26);
  closeValve(27);

  delay(delay_closeMBVs);

  // Open vents
  openValve(30);
  openValve(31);
}


void loop() {

    set485Listen();
  
    if (RS485Serial.available()) {
        set485Listen();
        String command = RS485Serial.readStringUntil('\n');
        command.trim();

        if (command.length() > 0) {

          char cmd = command[0];
          
          // Detect input
          if(cmd >= 'A' && cmd <= 'M' && currentHFstate == 0 )
            switch (cmd) {
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
          }
          else if(currentHFstate == 0 && cmd == '1'){
            // Enter HF mode
            HF0toHF1();
            currentHFstate = 1;
          }
          else if(currentHFstate == 1){
            if(cmd == '0'){
              HF1toHF0();
              currentHFstate = 0;
            }
            else if(cmd == '2'){
              HF1toHF2();
              currentHFstate = 2;
            }
          }
          else if(currentHFstate == 2){
            if(cmd == '0'){
              HF2toHF0();
              currentHFstate = 0;
            }
            else if(cmd == ''1){
              HF2toHF1();
              currentHFstate = 1;
            }
            else if(cmd == '3'){
              currentHFstate = 3;
            }
          }
          else if(currentHFstate == 3){
            if(cmd == 'N'){
              currentHFstate = 2; // Send back with no change
            }
            else if(cmd == 'Y'){
              HF2toHF3();
              engineFired = 1;
            }
            else if(cmd == '4' && engineFired == 1){
              HF3toHF4();
              engineFired = 0;
              currentHFstate = 4;
            }

          }
          else if(currentHFstate == 4){
            if(cmd == '5'){
              HF4toHF5();
              currentHFState = 0;
            }
          }
          
          
          RS485Serial.flush();
          delay(2);
          set485Listen();
          }
      }
    }
}
