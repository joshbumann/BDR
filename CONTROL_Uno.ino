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
bool statevec[8] = {0,0,0,0,0,0,0,0};



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
      
      if(statevec[i] == 0){
        stateStr += "CLOSED";
      }
      else if(statevec[i] == 1){
        stateStr += "OPEN";
      }     

      Serial.println(stateStr);
  }
}


// HOTFIRE FUNCTIONS

void HF0toHF1(){
  // Tank press

  statevec[0] = 1;
  statevec[1] = 1;
  statevec[2] = 0;
  statevec[3] = 0;
  statevec[4] = 0;
  statevec[5] = 0;
  statevec[6] = 0;
  statevec[7] = 0;

  transmitCommand('1');

  Serial.println("Pressurizing tanks.");
  Serial.println("Verify correct PT data before proceeding.");
  Serial.println("Available inputs:");
  Serial.println("0) HARD ABORT");
  Serial.println("S) Return to coldflow (TANKS WILL STAY PRESSURIZED)");
  Serial.println("2) HFS2: FIRE IGNITER");
  Serial.println("Z) Manual LOX tank bleed");
}
void HF1toHF2(){
  statevec[0] = 1;
  statevec[1] = 1;
  statevec[2] = 0;
  statevec[3] = 0;
  statevec[4] = 0;
  statevec[5] = 0;
  statevec[6] = 0;
  statevec[7] = 0;

  // Fire igniter
  transmitCommand('2');

  Serial.println("Ignition command sent");
  Serial.println("Available inputs:");
  Serial.println("0) HARD ABORT");
  Serial.println("1) Step back to HFS1");
  Serial.println("3) OPEN MAIN PROPELLANT VALVES");
}
void HF2toHF3(){
  // Open main prop valves

  statevec[0] = 1;
  statevec[1] = 1;
  statevec[2] = 1;
  statevec[3] = 1;
  statevec[4] = 0;
  statevec[5] = 0;
  statevec[6] = 0;
  statevec[7] = 0;
  
  transmitCommand('3');

  Serial.println("MPV command sent");
  Serial.println("Available inputs:");
  Serial.println("4) END TEST");

}
void HF3toHF4(){
  // End test
  statevec[0] = 1;
  statevec[1] = 1;
  statevec[2] = 0;
  statevec[3] = 0;
  statevec[4] = 0;
  statevec[5] = 0;
  statevec[6] = 1;
  statevec[7] = 1;

  transmitCommand('4');

  Serial.println("Ending test");
  Serial.println("MPVs Closed");
  Serial.println("Purge open");
  Serial.println("MBVs open");
}
void HF1toHF0(){
  // Hard abort: Close MBVs, open purge, and open vent simultaneously
  
  statevec[0] = 0;
  statevec[1] = 0;
  statevec[2] = 0;
  statevec[3] = 0;
  statevec[4] = 1;
  statevec[5] = 1;
  statevec[6] = 1;
  statevec[7] = 1;

  transmitCommand('0');
  Serial.println("HARD ABORT");

}
void HF2toHF0(){
  // Hard abort: Close MBVs, open purge, and open vent simultaneously
  
  statevec[0] = 0;
  statevec[1] = 0;
  statevec[2] = 0;
  statevec[3] = 0;
  statevec[4] = 1;
  statevec[5] = 1;
  statevec[6] = 1;
  statevec[7] = 1;

  transmitCommand('0');
  Serial.println("HARD ABORT");
}
void HF2toHF1(){
  // Return to tank press
  statevec[0] = 1;
  statevec[1] = 1;
  statevec[2] = 0;
  statevec[3] = 0;
  statevec[4] = 0;
  statevec[5] = 0;
  statevec[6] = 0;
  statevec[7] = 0;

  transmitCommand('1');
  Serial.println("Pressurizing tanks.");
  Serial.println("Verify correct PT data before proceeding.");
  Serial.println("Available inputs:");
  Serial.println("0) HARD ABORT");
  Serial.println("S) Return to coldflow (TANKS WILL STAY PRESSURIZED)");
  Serial.println("2) HFS2: FIRE IGNITER");
  Serial.println("Z) Manual LOX tank bleed");
}
void bleedLoxPress(){
  transmitCommand('Z');
}
void softAbort(){
  // Change nothing, leave up to CF

  statevec[0] = 1;
  statevec[1] = 1;
  statevec[2] = 0;
  statevec[3] = 0;
  statevec[4] = 0;
  statevec[5] = 0;
  statevec[6] = 0;
  statevec[7] = 0;

  transmitCommand('S');
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

    if(currentHFstate == 0){
      Serial.println("Available valves: ");

      Serial.println("A) Fuel N2 Valve"); // pressurization valves
      Serial.println("B) LOX N2 Valve"); // these might need to chars for the options
      Serial.println("C) Main Fuel Valve");
      Serial.println("D) Main Oxidizer Valve");
      Serial.println("E) Fuel Vent");
      Serial.println("F) LOX Vent");
      Serial.println("G) Fuel Purge"); // purge valves
      Serial.println("H) LOX Purge");
      Serial.println("I) Toggle Main Valves");
      Serial.println("J) Toggle Motorized(N2) Valves");
      Serial.println("K) Toggle vent valves");
      Serial.println("L) Toggle purge valves");
      Serial.println("M) Close all valves");
      Serial.println("Q) Check Valve States");
      Serial.println("1) Enter hotfire mode");


      Serial.println("Enter the option of the valve you want to toggle: ");
    }
    char cmd = getInput();

      if (cmd) {
        // Input logic
          if(cmd >= 'A' && cmd <= 'M' && currentHFstate == 0){
            transmitCommand(cmd);

            switch(cmd){   // Keep track of states
              case 'A':
                statevec[0] = !statevec[0];
              break;
              case 'B':
                statevec[1] = !statevec[1];
              break;
              case 'C':
                statevec[2] = !statevec[2];
              break;
              case 'D':
                statevec[3] = !statevec[3];
              break;
              case 'E':
                statevec[4] = !statevec[4];
              break;
              case 'F':
                statevec[5] = !statevec[5];
              break;
              case 'G':
                statevec[6] = !statevec[6];
              break;
              case 'H':
                statevec[7] = !statevec[7];
              break;
              case 'I':
                statevec[2] = !statevec[2];
                statevec[3] = !statevec[3];
              break;
              case 'J':
                statevec[0] = !statevec[0];
                statevec[1] = !statevec[1];
              break;
              case 'K':
                statevec[4] = !statevec[4];
                statevec[5] = !statevec[5];
              break;
              case 'L':
                statevec[6] = !statevec[6];
                statevec[7] = !statevec[7];
              break;
              case 'M':
                for(int i = 0; i < 8; i++){
                  statevec[i] = 0;
                }
                break;
              }
            
          }
          else if(cmd == 'Q'){
            stateCheck();
          }
          else if((cmd >= '0' && cmd <= '5') || cmd == 'Z' || cmd == 'S'){
            if(currentHFstate == 0 && cmd == '1'){
              // Enter HF mode

              HF0toHF1();
              currentHFstate = 1;
            }
            else if(currentHFstate == 1){ // Handle HFS 1
              if(cmd == '0'){
                // Hard abort
                
                HF1toHF0();
                currentHFstate = 0;
              }
              else if(cmd == 'S'){
                // Soft abort

                softAbort();
                currentHFstate = 0;
              }
              else if(cmd == '2'){
                // Fire igniter
                HF1toHF2();
                currentHFstate = 2;
              }
              else if(cmd == 'Z'){
                bleedLoxPress();
              }
              else{
                invalidInput();
              }
            }
            else if(currentHFstate == 2){ // Handle HFS 2
              if(cmd == '0'){
                // Hard abort
                HF2toHF0();
                currentHFstate = 0;
              }
              else if(cmd == '1'){
                // go from state 2 to state 1
                HF2toHF1();
                currentHFstate = 1;
              }
              else if(cmd == '3'){
                // go from state 2 to state 3
                HF2toHF3();
                currentHFstate = 3;
              }
              else{
                invalidInput();
              }
            }
            else if(currentHFstate == 3){         
                if(cmd == '4'){
                  HF3toHF4();
                  currentHFstate = 0;
                }
                else{
                  invalidInput();
                }
            }
           
            
          }
        }
  }
}
