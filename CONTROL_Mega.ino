// V2.10
// For second round of static fires


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
uint8_t relayMask = 0;
bool sequenceTriggered[5] = {0,0,0,0,0}; // Track which sequences have been triggered (1-5)

enum CommandCode : char {
  CMD_FUEL_N2 = 'A',
  CMD_LOX_N2 = 'B',
  CMD_MAIN_FUEL = 'C',
  CMD_MAIN_OX = 'D',
  CMD_FUEL_VENT = 'E',
  CMD_LOX_VENT = 'F',
  CMD_FUEL_PURGE = 'G',
  CMD_LOX_PURGE = 'H',
  CMD_TOGGLE_MAIN = 'I',
  CMD_TOGGLE_N2 = 'J',
  CMD_TOGGLE_VENTS = 'K',
  CMD_TOGGLE_PURGE = 'L',
  CMD_CLOSE_ALL = 'M',
  CMD_HARD_ABORT = '0',
  CMD_ENTER_HOTFIRE = '1',
  CMD_FIRE_IGNITER = '2',
  CMD_OPEN_MAIN_PROPELLANTS = '3',
  CMD_END_TEST = '4',
  CMD_BLEED_LOX = 'Z',
  CMD_SOFT_ABORT = 'S'
};

struct CommandOption {
  char cmd;
  const char* label;
};

const CommandOption commandMap[] = {
  {CMD_FUEL_N2, "Fuel N2 Valve"},
  {CMD_LOX_N2, "LOX N2 Valve"},
  {CMD_MAIN_FUEL, "Main Fuel Valve"},
  {CMD_MAIN_OX, "Main Oxidizer Valve"},
  {CMD_FUEL_VENT, "Fuel Vent"},
  {CMD_LOX_VENT, "LOX Vent"},
  {CMD_FUEL_PURGE, "Fuel Purge"},
  {CMD_LOX_PURGE, "LOX Purge"},
  {CMD_TOGGLE_MAIN, "Toggle Main Valves"},
  {CMD_TOGGLE_N2, "Toggle Motorized(N2) Valves"},
  {CMD_TOGGLE_VENTS, "Toggle vent valves"},
  {CMD_TOGGLE_PURGE, "Toggle purge valves"},
  {CMD_CLOSE_ALL, "Close all valves"},
  {CMD_ENTER_HOTFIRE, "Enter hotfire mode"}
};

const int NUM_VALVES = 8;

const int igniterPin = 54;

const int Fuel_N2_Vlv_Pin = 26;  // Fuel N2
const int LOX_N2_Vlv_Pin = 27;  // LOX N2
const int MFV_VLv_Pin = 28;   // MFV
const int MOV_Vlv_Pin = 29;   // MOV
const int Fuel_Vent_Vlv_Pin = 30;   // Fuel Vent
const int LOX_Vent_Vlv_Pin = 31;   // LOX Vent
const int Fuel_Purge_Vlv_Pin = 32;   // Fuel Purge
const int LOX_Purge_Vlv_Pin = 33;   // LOX Purge

inline void set485Listen() {          // RE=LOW, DE=LOW
  digitalWrite(RS485_RE_PIN, LOW);
  digitalWrite(RS485_DE_PIN, LOW);
}
inline void set485Talk() {            // RE=HIGH, DE=HIGH
  digitalWrite(RS485_RE_PIN, HIGH);
  digitalWrite(RS485_DE_PIN, HIGH);
}

inline void setRelayState(uint8_t idx, bool on) {
  if (on) {
    relayMask |= (1u << idx);
  } else {
    relayMask &= ~(1u << idx);
  }

  uint8_t lvl = ACTIVE_LOW ? (on ? LOW : HIGH) : (on ? HIGH : LOW);
  digitalWrite(RELAY_PINS[idx], lvl);
}
inline bool getRelayState(uint8_t idx) {
  return (relayMask & (1u << idx)) != 0;
}
inline void toggleRelay(uint8_t idx) { setRelayState(idx, !getRelayState(idx)); }

// Turn a specific valve ON
void openValve(int valveIndex) {
  if (valveIndex == Fuel_Vent_Vlv_Pin || valveIndex == LOX_Vent_Vlv_Pin || valveIndex == Fuel_Purge_Vlv_Pin || valveIndex == LOX_Purge_Vlv_Pin) {
    // For vent and purge valves, HIGH means closed, LOW means open
    digitalWrite(valveIndex, LOW);
  } else {
    // For other valves, HIGH means open, LOW means closed
    digitalWrite(valveIndex, HIGH);
  }
}

// Turn a specific valve OFF
void closeValve(int valveIndex) {
  if (valveIndex == Fuel_Vent_Vlv_Pin || valveIndex == LOX_Vent_Vlv_Pin || valveIndex == Fuel_Purge_Vlv_Pin || valveIndex == LOX_Purge_Vlv_Pin) {
    // For vent and purge valves, HIGH means closed, LOW means open
    digitalWrite(valveIndex, HIGH);
  } else {
    // For other valves, HIGH means open, LOW means closed
    digitalWrite(valveIndex, LOW);
  }
}

// All valves close
void closeAllValves() {
    relayMask = 0;
    digitalWrite(Fuel_N2_Vlv_Pin, LOW);
    digitalWrite(LOX_N2_Vlv_Pin, LOW);
    digitalWrite(MFV_VLv_Pin, LOW);
    digitalWrite(MOV_Vlv_Pin, LOW);
    digitalWrite(Fuel_Vent_Vlv_Pin, HIGH); // Vents must be high to be closed
    digitalWrite(LOX_Vent_Vlv_Pin, HIGH); //
    digitalWrite(Fuel_Purge_Vlv_Pin, HIGH); // Purge too
    digitalWrite(LOX_Purge_Vlv_Pin, HIGH);
}

// Toggle valve from current state
void toggleValve(int valveIndex) {
  if (valveIndex >= 26 && valveIndex <= 33) {
    uint8_t idx = (uint8_t)(valveIndex - 26);
    setRelayState(idx, !getRelayState(idx));
  }
}

// HOTFIRE VARIABLES

int currentHFstate = 0; // Keeps track of current state (0-5). 0 means coldflow mode.
bool engineFired = 0;   // Helps with logic for ending hot fire. Changes if engine has been fired.
bool igniterFired = 0;

int delay_closevent_openMBVs = 10; // HFS2 : the delay betewen closing the vents and opening the MBVs. Something small just to act as a buffer not to vent any unessesary N2
int delay_openMBVs = 15000;           // This is the standard time it takes for the MBVs to fully open. Having this delay in the code will safeguard against trying to do anything unless they are fully toggled.
int delay_closeMBVs = 15000;          // Same as above, but in case closing takes a different time.
int delay_toggleVents = 0;        // This delay will go between opening and closing the vents. Just needs to be the amount of time to depressurize.

int delay_MFV_MOV = 20;            // This is the delay between opening the main valves. We want the liquids to enter the injector at the same time, and this takes into account that the fuel needs to travel through the regen channels.
//int delay_igniter = 0;            // Delay between igniter firing and the main propellant valves opening.

int delay_Bleed = 1000;
int delay_closeMVs_openPurge = 0;       // Delay between closing the main valves and opening the the purge and vent lines
int delay_purgeEnd = 5000;              // How long the purge is open to confirm shutdown
int delay_closePurge_closeVents = 0;    // Short delay between closing the purge and vents so everything the the purge lines can escape

//int TEST_LENGTH = 3000;         // How long the MPVs will be open for a given test (UNUSED IN CURRENT VERSION)

// HOTFIRE FUNCTIONS

void HF0toHF1(){
  // Tank press

  // Close vents, purge, and MPVs
  closeValve(MFV_VLv_Pin); // MPVs
  closeValve(MOV_Vlv_Pin);
  closeValve(Fuel_Vent_Vlv_Pin); // Vents
  closeValve(LOX_Vent_Vlv_Pin);
  closeValve(Fuel_Purge_Vlv_Pin); // Purge
  closeValve(LOX_Purge_Vlv_Pin);

  // Small delay to ensure vent close
  delay(delay_closevent_openMBVs);

  // Open MBVs
  openValve(Fuel_N2_Vlv_Pin);
  openValve(LOX_N2_Vlv_Pin);
  // The MBVs being fully open is up to the user to determine
}
void HF1toHF2(){
  // Fire igniter
  digitalWrite(igniterPin, HIGH);
}
void HF2toHF3(){
  // Open main prop valves, w/ delay for regen channels
  openValve(MFV_VLv_Pin);
  delay(delay_MFV_MOV);
  openValve(MOV_Vlv_Pin);

  // Write low to igniter pin
  digitalWrite(igniterPin,LOW);
}
void HF3toHF4(){
  // End test

  // Open purge
  openValve(Fuel_Purge_Vlv_Pin);
  openValve(LOX_Purge_Vlv_Pin);

  // Close MPVs 
  closeValve(MFV_VLv_Pin);
  closeValve(MOV_Vlv_Pin);
}
void HF1toHF0(){
  // Hard abort: Close MBVs, open purge, and open vent simultaneously
  // Close MBVs
  closeValve(Fuel_N2_Vlv_Pin);
  closeValve(LOX_N2_Vlv_Pin);

  // Open vents
  openValve(Fuel_Vent_Vlv_Pin);
  openValve(LOX_Vent_Vlv_Pin);

  // Open purge
  openValve(Fuel_Purge_Vlv_Pin);
  openValve(LOX_Purge_Vlv_Pin);
}
void HF2toHF0(){
  // Hard abort: Close MBVs, open purge, and open vent simultaneously
  // Close MBVs
  closeValve(Fuel_N2_Vlv_Pin);
  closeValve(LOX_N2_Vlv_Pin);

  // Open vents
  openValve(Fuel_Vent_Vlv_Pin);
  openValve(LOX_Vent_Vlv_Pin);

  // Open purge
  openValve(Fuel_Purge_Vlv_Pin);
  openValve(LOX_Purge_Vlv_Pin);

  // Write low to igniter pin
  digitalWrite(igniterPin,LOW);
}
void HF2toHF1(){
  // Write low to igniter pin
  digitalWrite(igniterPin,LOW);
}
void bleedLoxPress(){
  openValve(LOX_Vent_Vlv_Pin);
  delay(delay_Bleed);
  closeValve(LOX_Vent_Vlv_Pin);
}
void softAbort(){
  // Change nothing, leave up to CF
}

void setup() {
  // Initialize RS485 communication (SoftwareSerial)
  //Serial.begin(115200);

  pinMode(RS485_RE_PIN, OUTPUT);
  pinMode(RS485_DE_PIN, OUTPUT);
  set485Listen();                       // idle in listen

  RS485Serial.begin(57600);
  // Make sure all pnumatic valves start OFF
  for (uint8_t i = 0; i < 8; ++i) {
    pinMode(RELAY_PINS[i], OUTPUT);
    setRelay(i, false);
  }
 closeAllValves();
 pinMode(igniterPin, OUTPUT);

}

void loop() {

    set485Listen();
  
    if (RS485Serial.available()) {
        set485Listen();
        String command = RS485Serial.readStringUntil('\n');
        command.trim();

        if (command.length() > 0) {

          char cmd = command[0];
          
          // Sort input
          if(cmd >= CMD_FUEL_N2 && cmd <= CMD_CLOSE_ALL && currentHFstate == 0 ){
            switch (cmd) {
              case CMD_FUEL_N2:
                toggleValve(Fuel_N2_Vlv_Pin);
                break;
              case CMD_LOX_N2:
                toggleValve(LOX_N2_Vlv_Pin);
                break;
              case CMD_MAIN_FUEL:
                toggleValve(MFV_VLv_Pin);
                break;
              case CMD_MAIN_OX:
                toggleValve(MOV_Vlv_Pin);
                break;
              case CMD_FUEL_VENT:
                toggleValve(Fuel_Vent_Vlv_Pin);
                break;
              case CMD_LOX_VENT:
                toggleValve(LOX_Vent_Vlv_Pin);
                break;
              case CMD_FUEL_PURGE:
                toggleValve(Fuel_Purge_Vlv_Pin);
                break;
              case CMD_LOX_PURGE:
                toggleValve(LOX_Purge_Vlv_Pin);
                break;
              case CMD_TOGGLE_MAIN:
                toggleValve(MFV_VLv_Pin); // MFV
                toggleValve(MOV_Vlv_Pin); // MOV
                break;
              case CMD_TOGGLE_N2:
                toggleValve(Fuel_N2_Vlv_Pin); // N2 valves
                toggleValve(LOX_N2_Vlv_Pin);
                break;
              case CMD_TOGGLE_VENTS:
                toggleValve(Fuel_Vent_Vlv_Pin); // Vent valves
                toggleValve(LOX_Vent_Vlv_Pin);
                break;
              case CMD_TOGGLE_PURGE:
                toggleValve(Fuel_Purge_Vlv_Pin); // Purge valves
                toggleValve(LOX_Purge_Vlv_Pin);
                break;
              case CMD_CLOSE_ALL:
                closeAllValves();
                break;
              default:
                //Serial.println("Invalid input. Enter 1-8.");
              break;
            }
          }
          else if(currentHFstate == 0 && cmd == CMD_ENTER_HOTFIRE){
            // Enter HF mode
            HF0toHF1();
            currentHFstate = 1;
          }
          else if(currentHFstate == 1){
            if(cmd == CMD_HARD_ABORT){
              HF1toHF0();
              currentHFstate = 0;
            }
            else if(cmd == CMD_FIRE_IGNITER){
              HF1toHF2();
              currentHFstate = 2;
              igniterFired = 1;
            }
            else if(cmd == CMD_SOFT_ABORT){
              softAbort();
              currentHFstate = 0;
            }
            else if(cmd == CMD_BLEED_LOX){
              bleedLoxPress();
            }
          }
          else if(currentHFstate == 2){
            if(cmd == CMD_HARD_ABORT){
              HF2toHF0();
              currentHFstate = 0;
            }
            else if(cmd == CMD_ENTER_HOTFIRE){
              HF2toHF1();
              currentHFstate = 1;
            }
            else if(cmd == CMD_OPEN_MAIN_PROPELLANTS){
              HF2toHF3();
              currentHFstate = 3;
              engineFired = 1;
            }
          }
          else if(currentHFstate == 3){          
            if(cmd == CMD_END_TEST){
              HF3toHF4();
              currentHFstate = 0;
              // ends test
            }
          }
          
          
          RS485Serial.flush();
          delay(2);
          set485Listen();
          }
      }
    }
1