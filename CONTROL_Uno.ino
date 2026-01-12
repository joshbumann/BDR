#include <SoftwareSerial.h>

// RS-485 pins on UNO
const uint8_t RS485_RO_PIN = 4;  // RX from MAX485 RO
const uint8_t RS485_RE_PIN = 5;  // RE (LOW = listen, HIGH = talk)
const uint8_t RS485_DE_PIN = 6;  // DE (LOW = listen, HIGH = talk)
const uint8_t RS485_DI_PIN = 7;  // TX to MAX485 DI

SoftwareSerial RS485Serial(RS485_RO_PIN, RS485_DI_PIN); // rx, tx

bool hotfireMode = 0;
bool statevec[8] = {0,0,0,0,0,0,0,0};
int currentHFState = 0; // Track which sequences have been triggered (1-5)

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
  delay(2);              // small guard at 9600 bps
  set485Listen();
}

void invalidInput() {
  Serial.println("Invalid input. Please try again.");
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

void setup() {
  Serial.begin(115200);
  pinMode(RS485_RE_PIN, OUTPUT);
  pinMode(RS485_DE_PIN, OUTPUT);
  
  // Initialize RS485 communication (SoftwareSerial)
  
  set485Listen();              // idle in listen
  RS485Serial.begin(9600);     // must match MEGA
  
  Serial.println(F("UNO RS485 Transmitter ready."));
}

char getInput(){
  string command = Serial.readStringUntil('\n'); // Read until a newline character
    command.trim(); // Remove any whitespace
    
    char cmd = 0;
   
        for (uint16_t i = 0; i < command.length(); ++i) {
          char c = command[i];
          if ((c >= 'A' && c <= 'N') || (c >= '0' && c <= '5') || c == 'Y' || c == 'Q') { cmd = c; break; }
        }
        return cmd;
}


void loop() {

  if (Serial.available()) {

    if(hotfireMode == 0){
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


      Serial.println("Enter the option of the valve you want to toggle: ");
    }
    else if(hotfireMode == 1){
      Serial.println("HOTFIRE MODE");
      Serial.println("0) Return to coldflow");
      Serial.println("1) Load Tanks");
      Serial.println("2) Pressurize Tanks");
      Serial.println("3) Begin Ignition Sequence (Requires confirmation)");
      Serial.println("4) End/Abort Fire");
      Serial.println("5) End Test - Return to Coldflow Mode");
      Serial.println("Y) Confirm Command");
      Serial.println("N) Cancel Command");
      Serial.println("Q) Check Valve States (Coming soon!)");


      Serial.println("Enter command: ");
    }
    cmd = getInput();
        
    
        if (cmd) {
          // Transmitted input
          if(cmd >= 'A' && cmd <= 'M' && hotfireMode == 0){
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
                statevec[4] = 1;
                statevec[5] = 1;
                break;
              }
            
          }
          else if(cmd == 'Q'){
            stateCheck();
          }
          else if((cmd >= '0' && cmd <= '5') || cmd == 'Y' || cmd == 'N'){
            if(hotfireMode == 0 && cmd == '1'){
              // enter hotfire mode
              // currentHFState = 1
              // transmit 1 to the mega
              
              hotfireMode = 1;
              currentHFState = 1;
              transmitCommand('1');

              Serial.println("Entered hotfire mode.");
              Serial.println("Available inputs:");
              Serial.println("0) Return to coldflow state");
              Serial.println("2) HFS2: Pressurize tanks");
            }
            else if(hotfireMode == 1 && currentHFState == 1){ // Handle HFS 1
              if(cmd == '0'){
                // go from state 1 to state 0

                // exit hotfire mode
                // currentHFState = 0
                // transmit 0 to the mega
                
                hotfireMode = 0;
                currentHFState = 0;
                transmitCommand('0');
              }
              else if(cmd == '2'){
                // go from state 1 to state 2

                // currentHFState = 2
                // transmit 2 to the mega
                
                currentHFState = 2;
                transmitCommand('2');

                Serial.println("Pressurizing tanks.");
                Serial.println("Verify correct PT data before proceeding.");
                Serial.println("Available inputs:");
                Serial.println("0) Return to coldflow state");
                Serial.println("1) HFS1: Depressurize tanks");
                Serial.println("3) HFS3: BEGIN IGNITION SEQUENCE (Requires confirmation)");
              }
              else{
                invalidInput();
              }
            }
            else if(hotfireMode == 1 && currentHFState == 2){ // Handle HFS 2
              if(cmd == '0'){
                // go from state 2 to state 0

                // exit hotfire mode
                // currentHFState = 0
                // transmit 0 to the mega
                
                hotfireMode = 0;
                currentHFState = 0;
                transmitCommand('0'); // MEGA NEEDS TO HANDLE 
              }
              else if(cmd == '1'){
                // go from state 2 to state 1

                // currentHFState = 1
                // transmit 1 to the mega
                
                currentHFState = 1;
                transmitCommand('1');

                Serial.println("Tanks depressurizing");
                Serial.println("Verify with PT data before proceeding");
                Serial.println("Available inputs:");
                Serial.println("0) Return to coldflow state");
                Serial.println("2) HFS2: Pressurize tanks");
              }
              else if(cmd == '3'){
                // go from state 2 to state 3
                currentHFState = 3;
                transmitCommand('3');

                Serial.println("Confirm ignition sequence (Y/N)");
                cmdHF = getInput();
                if(cmdHF == 'N'){
                  transmitCommand('N');
                  currentHFState = 2;
                  transmitCommand('2');
                }
                else if(cmdHF == 'Y'){
                  transmitCommand('Y');
                  Serial.println("Available inputs:");
                  Serial.println("4) ABORT / end test");
                }
              }
              else{
                invalidInput();
              }
            }
            else if(hotfireMode == 1 && currentHFState == 3){ // Handle HFS 3 (Cant go back from here)
              if(cmd == '4'){
                // currentHFState = 4
                // transmit 4 to the mega
                
                currentHFState = 4;
                transmitCommand('4');

                Serial.println("Test over.");
                Serial.println("Available inputs:");
                Serial.println("5) Return to coldflow state");
              }
              else{
                invalidInput();
              }

            }
            else if(hotfireMode == 1 && currentHFState == 4){ // Handle HFS 4
              if(cmd == '5'){
                // go from state 4 to state 5 (End test)

                // hotfireMode off
                // currentHFState = 0
                // transmit 5 to the mega
                
                hotfireMode = 0;
                currentHFState = 0;
                transmitCommand('5');
              }
              else{
                invalidInput();
              }
              
            }
            else{
              invalidInput();
            }
            
        }
        else {
          invalidInput();
        }
  }
}
