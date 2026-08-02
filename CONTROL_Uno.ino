// V2.10

// For second round of static fires

#include <SoftwareSerial.h>

// RS-485 pins on UNO
const uint8_t RS485_RO_PIN = 4;  // RX from MAX485 RO
const uint8_t RS485_RE_PIN = 5;  // RE (LOW = listen, HIGH = talk)
const uint8_t RS485_DE_PIN = 6;  // DE (LOW = listen, HIGH = talk)
const uint8_t RS485_DI_PIN = 7;  // TX to MAX485 DI

SoftwareSerial RS485Serial(RS485_RO_PIN, RS485_DI_PIN); // rx, tx

int currentHFstate = 0; // Keeps track of current state (0-5). 0 means coldflow mode.

enum HFState {
  COLD_FLOW = 0,
  TANK_PRESS = 1,
  IGNITION = 2,
  MAIN_BURN = 3,
  END_TEST = 4
};

// Single-byte bitmask for the 8 valve states: bit 0 = Fuel N2, bit 1 = LOX N2, ...
uint8_t stateMask = 0;

inline void setValveState(uint8_t index, bool open) {
  if (open) {
    stateMask |= (1u << index);
  } else {
    stateMask &= ~(1u << index);
  }
}

inline bool getValveState(uint8_t index) {
  return (stateMask & (1u << index)) != 0;
}



// I/O Functions
inline void set485Listen() {   // Receive mode: RE=LOW, DE=LOW
  digitalWrite(RS485_RE_PIN, LOW);
  digitalWrite(RS485_DE_PIN, LOW);
}
inline void set485Talk() {     // Transmit mode: RE=HIGH, DE=HIGH
  digitalWrite(RS485_RE_PIN, HIGH);
  digitalWrite(RS485_DE_PIN, HIGH);
}
void transmitCommand(char cmd) {
  set485Talk();
  RS485Serial.write(cmd);
  RS485Serial.write('\n');
  RS485Serial.flush();   // ensure bytes left TX buffer
  delay(2);              // small guard
  set485Listen();
}
void invalidInput() {
  Serial.println("Invalid input. Please try again.");
}
char getInput(){
  String command = Serial.readStringUntil('\n'); // Read until a newline character
    command.trim(); // Remove any whitespace
    char cmd = 0;
   
        for (uint16_t i = 0; i < command.length(); ++i) {
          char c = command[i];
          if ((c >= 'A' && c <= 'M') || (c >= '0' && c <= '5') || c == 'Q' || c == 'Z' || c == 'S') { cmd = c; break; }
        }
        return cmd;
}

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
  CMD_CHECK_STATES = 'Q',
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

const CommandOption coldFlowCommands[] = {
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
  {CMD_CHECK_STATES, "Check Valve States"},
  {CMD_ENTER_HOTFIRE, "Enter hotfire mode"}
};

void stateCheck(){
  String stateStr = "";
  
  for(int i = 0; i<8; i++){
      stateStr = "";
      
      switch(i){
        case 0:
          stateStr = "Fuel N2 Valve:    ";   
  
        break;
        case 1:
          stateStr = "LOX N2 Valve:     ";
  
        break;
        case 2:
          stateStr = "Main Fuel Valve:  ";
  
        break;
        case 3:
          stateStr = "Main O2 Valve:    ";
          
        break;
        case 4:
          stateStr = "Fuel Vent Valve:  ";
          
        break;
        case 5:
          stateStr = "O2 Vent Valve:    ";
  
        break;
        case 6:
          stateStr = "Fuel Purge Valve: ";
  
        break;
        case 7:
          stateStr = "O2 Purge Valve:   ";
        break;
        default:
        break;
      }
      
      if(!getValveState(i)){
        stateStr += "CLOSED";
      }
      else {
        stateStr += "OPEN";
      }     

      Serial.println(stateStr);
  }
}


// HOTFIRE FUNCTIONS

void HF0toHF1(){
  // Tank press

  setValveState(0, true);
  setValveState(1, true);
  setValveState(2, false);
  setValveState(3, false);
  setValveState(4, false);
  setValveState(5, false);
  setValveState(6, false);
  setValveState(7, false);

  transmitCommand(CMD_ENTER_HOTFIRE);

  Serial.println("Pressurizing tanks.");
  Serial.println("Verify correct PT data before proceeding.");
  Serial.println("Available inputs:");
  Serial.println("0) HARD ABORT");
  Serial.println("S) Return to coldflow (TANKS WILL STAY PRESSURIZED)");
  Serial.println("2) HFS2: FIRE IGNITER");
  Serial.println("Z) Manual LOX tank bleed");
}
void HF1toHF2(){
  setValveState(0, true);
  setValveState(1, true);
  setValveState(2, false);
  setValveState(3, false);
  setValveState(4, false);
  setValveState(5, false);
  setValveState(6, false);
  setValveState(7, false);

  // Fire igniter
  transmitCommand(CMD_FIRE_IGNITER);

  Serial.println("Ignition command sent");
  Serial.println("Available inputs:");
  Serial.println("0) HARD ABORT");
  Serial.println("1) Step back to HFS1");
  Serial.println("3) OPEN MAIN PROPELLANT VALVES");
}
void HF2toHF3(){
  // Open main prop valves

  setValveState(0, true);
  setValveState(1, true);
  setValveState(2, true);
  setValveState(3, true);
  setValveState(4, false);
  setValveState(5, false);
  setValveState(6, false);
  setValveState(7, false);
  
  transmitCommand(CMD_OPEN_MAIN_PROPELLANTS);

  Serial.println("MPV command sent");
  Serial.println("Available inputs:");
  Serial.println("4) END TEST");

}
void HF3toHF4(){
  // End test
  setValveState(0, true);
  setValveState(1, true);
  setValveState(2, false);
  setValveState(3, false);
  setValveState(4, false);
  setValveState(5, false);
  setValveState(6, true);
  setValveState(7, true);

  transmitCommand(CMD_END_TEST);

  Serial.println("Ending test");
  Serial.println("MPVs Closed");
  Serial.println("Purge open");
  Serial.println("MBVs open");
}
void HF1toHF0(){
  // Hard abort: Close MBVs, open purge, and open vent simultaneously
  
  setValveState(0, false);
  setValveState(1, false);
  setValveState(2, false);
  setValveState(3, false);
  setValveState(4, true);
  setValveState(5, true);
  setValveState(6, true);
  setValveState(7, true);

  transmitCommand(CMD_HARD_ABORT);
  Serial.println("HARD ABORT");

}
void HF2toHF0(){
  // Hard abort: Close MBVs, open purge, and open vent simultaneously
  
  setValveState(0, false);
  setValveState(1, false);
  setValveState(2, false);
  setValveState(3, false);
  setValveState(4, true);
  setValveState(5, true);
  setValveState(6, true);
  setValveState(7, true);

  transmitCommand(CMD_HARD_ABORT);
  Serial.println("HARD ABORT");
}
void HF2toHF1(){
  // Return to tank press
  setValveState(0, true);
  setValveState(1, true);
  setValveState(2, false);
  setValveState(3, false);
  setValveState(4, false);
  setValveState(5, false);
  setValveState(6, false);
  setValveState(7, false);

  transmitCommand(CMD_ENTER_HOTFIRE);
  Serial.println("Pressurizing tanks.");
  Serial.println("Verify correct PT data before proceeding.");
  Serial.println("Available inputs:");
  Serial.println("0) HARD ABORT");
  Serial.println("S) Return to coldflow (TANKS WILL STAY PRESSURIZED)");
  Serial.println("2) HFS2: FIRE IGNITER");
  Serial.println("Z) Manual LOX tank bleed");
}
void bleedLoxPress(){
  transmitCommand(CMD_BLEED_LOX);
}
void softAbort(){
  // Change nothing, leave up to CF

  setValveState(0, true);
  setValveState(1, true);
  setValveState(2, false);
  setValveState(3, false);
  setValveState(4, false);
  setValveState(5, false);
  setValveState(6, false);
  setValveState(7, false);

  transmitCommand(CMD_SOFT_ABORT);
  Serial.println("Soft abort: returning to coldflow");
}



void setup() {
  Serial.begin(115200);
  pinMode(RS485_RE_PIN, OUTPUT);
  pinMode(RS485_DE_PIN, OUTPUT);
  
  // Initialize RS485 communication (SoftwareSerial)
  
  set485Listen();              // idle in listen
  RS485Serial.begin(57600);     // must match MEGA
  
  Serial.println(F("UNO RS485 Transmitter ready."));
}


void loop() {

  if (Serial.available()) {

    if(currentHFstate == COLD_FLOW){
      Serial.println("Available valves: ");

      for (uint8_t i = 0; i < (sizeof(coldFlowCommands) / sizeof(coldFlowCommands[0])); ++i) {
        Serial.print(coldFlowCommands[i].cmd);
        Serial.print(") ");
        Serial.println(coldFlowCommands[i].label);
      }

      Serial.println("Enter the option of the valve you want to toggle: ");
    }
    char cmd = getInput();

      if (cmd) {
        // Input logic
          if((cmd >= CMD_FUEL_N2 && cmd <= CMD_CLOSE_ALL) && currentHFstate == COLD_FLOW){
            transmitCommand(cmd);

            switch(cmd){   // Keep track of states
              case CMD_FUEL_N2:
                setValveState(0, !getValveState(0));
              break;
              case CMD_LOX_N2:
                setValveState(1, !getValveState(1));
              break;
              case CMD_MAIN_FUEL:
                setValveState(2, !getValveState(2));
              break;
              case CMD_MAIN_OX:
                setValveState(3, !getValveState(3));
              break;
              case CMD_FUEL_VENT:
                setValveState(4, !getValveState(4));
              break;
              case CMD_LOX_VENT:
                setValveState(5, !getValveState(5));
              break;
              case CMD_FUEL_PURGE:
                setValveState(6, !getValveState(6));
              break;
              case CMD_LOX_PURGE:
                setValveState(7, !getValveState(7));
              break;
              case CMD_TOGGLE_MAIN:
                setValveState(2, !getValveState(2));
                setValveState(3, !getValveState(3));
              break;
              case CMD_TOGGLE_N2:
                setValveState(0, !getValveState(0));
                setValveState(1, !getValveState(1));
              break;
              case CMD_TOGGLE_VENTS:
                setValveState(4, !getValveState(4));
                setValveState(5, !getValveState(5));
              break;
              case CMD_TOGGLE_PURGE:
                setValveState(6, !getValveState(6));
                setValveState(7, !getValveState(7));
              break;
              case CMD_CLOSE_ALL:
                stateMask = 0;
                break;
              }
            
          }
          else if(cmd == CMD_CHECK_STATES){
            stateCheck();
          }
          else if((cmd >= '0' && cmd <= '5') || cmd == CMD_BLEED_LOX || cmd == CMD_SOFT_ABORT){
            if(currentHFstate == COLD_FLOW && cmd == CMD_ENTER_HOTFIRE){
              // Enter HF mode

              HF0toHF1();
              currentHFstate = TANK_PRESS;
            }
            else if(currentHFstate == TANK_PRESS){ // Handle HFS 1
              if(cmd == CMD_HARD_ABORT){
                // Hard abort
                
                HF1toHF0();
                currentHFstate = COLD_FLOW;
              }
              else if(cmd == CMD_SOFT_ABORT){
                // Soft abort

                softAbort();
                currentHFstate = COLD_FLOW;
              }
              else if(cmd == CMD_FIRE_IGNITER){
                // Fire igniter
                HF1toHF2();
                currentHFstate = IGNITION;
              }
              else if(cmd == CMD_BLEED_LOX){
                bleedLoxPress();
              }
              else{
                invalidInput();
              }
            }
            else if(currentHFstate == IGNITION){ // Handle HFS 2
              if(cmd == CMD_HARD_ABORT){
                // Hard abort
                HF2toHF0();
                currentHFstate = COLD_FLOW;
              }
              else if(cmd == CMD_ENTER_HOTFIRE){
                // go from state 2 to state 1
                HF2toHF1();
                currentHFstate = TANK_PRESS;
              }
              else if(cmd == CMD_OPEN_MAIN_PROPELLANTS){
                // go from state 2 to state 3
                HF2toHF3();
                currentHFstate = MAIN_BURN;
              }
              else{
                invalidInput();
              }
            }
            else if(currentHFstate == MAIN_BURN){         
                if(cmd == CMD_END_TEST){
                  HF3toHF4();
                  currentHFstate = COLD_FLOW;
                }
                else{
                  invalidInput();
                }
            }
           
            
          }
        }
  }
}
