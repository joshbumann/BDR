/// V2.9

// Used on first hotfire

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
bool ignFired = 0;
bool engFired = 0;

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
  String command = Serial.readStringUntil('\n'); // Read until a newline character
    command.trim(); // Remove any whitespace
    
    char cmd = 0;
   
        for (uint16_t i = 0; i < command.length(); ++i) {
          char c = command[i];
          if ((c >= 'A' && c <= 'M') || (c >= '0' && c <= '5') || c == 'Q' || c == 'Z') { cmd = c; break; }
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
      Serial.println("1) Enter hotfire mode");


      Serial.println("Enter the option of the valve you want to toggle: ");
    }
    char cmd = getInput();
        
    
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
                statevec[4] = 0;
                statevec[5] = 0;
                break;
              }
            
          }
          else if(cmd == 'Q'){
            stateCheck();
          }
          else if((cmd >= '0' && cmd <= '5')){
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
                
                statevec[0] = 0;
                statevec[1] = 0;
                statevec[2] = 0;
                statevec[3] = 0;
                statevec[4] = 1;
                statevec[5] = 1;
                statevec[6] = 0;
                statevec[7] = 0;

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
                Serial.println("0) HARD ABORT");
                Serial.println("1) HFS1: Depressurize tanks (You will be locked out of controls for 15 seconds)");
                Serial.println("3) HFS3: Fire igniter");
                Serial.println("Z) Manual LOX tank bleed");
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

                // NOTE: need to update state variables, returning to CF mode
                statevec[0] = 0;
                statevec[1] = 0;
                statevec[2] = 0;
                statevec[3] = 0;
                statevec[4] = 1;
                statevec[5] = 1;
                statevec[6] = 1;
                statevec[7] = 1;

                transmitCommand('0');
              }
              else if(cmd == '1'){
                // go from state 2 to state 1

                // currentHFState = 1
                // transmit 1 to the mega
                
                currentHFState = 1;
                transmitCommand('1');

                Serial.println("Closing MBVs");
                Serial.println("You are locked out of controls for 15 seconds");
                delay(15000);
                Serial.println("Tanks depressurized");
                Serial.println("Verify with PT data before proceeding");
                Serial.println("MBVs:  CLOSED");
                Serial.println("Vents: OPEN");
                Serial.println("Available inputs:");
                Serial.println("0) Return to coldflow state");
                Serial.println("2) HFS2: Pressurize tanks");
              }
              else if(cmd == '3'){
                // go from state 2 to state 3
                currentHFState = 3;
                transmitCommand('3');
              }
              else{
                invalidInput();
              }
            }
            else if(hotfireMode == 1 && currentHFState == 3){
              
                Serial.println("Available inputs:");
                Serial.println("0) Hard abort");
                Serial.println("2) Return to HFS2");
                Serial.println("4) Open MPVs  (requires confirmation)");
                  
                if(cmd == '4'){
                    // currentHFState = 4
                    // transmit 4 to the mega
                    currentHFState = 4;
                    transmitCommand('4');
                }
                else if(cmd == '0'){
                  // go from state 3 to state 0

                  // exit hotfire mode
                  // currentHFState = 0
                  // transmit 0 to the mega
                  
                  hotfireMode = 0;
                  currentHFState = 0;

                  // NOTE: need to update state variables, returning to CF mode
                  statevec[0] = 0;
                  statevec[1] = 0;
                  statevec[2] = 0;
                  statevec[3] = 0;
                  statevec[4] = 1;
                  statevec[5] = 1;
                  statevec[6] = 1;
                  statevec[7] = 1;

                  ignFired = 0;

                  transmitCommand('0');
                }
                else if(cmd == '2'){
                  currentHFState = 2;
                  ignFired = 0;
                  transmitCommand('2');

                  Serial.println("HFS2");
                  Serial.println("Available inputs:");
                  Serial.println("0) HARD ABORT");
                  Serial.println("1) HFS1: Depressurize tanks (Soft abort)");
                  Serial.println("3) HFS3: Fire igniter (Requires confirmation)");
                  Serial.println("Z) Manual LOX tank bleed");
                }
                else{
                  invalidInput();
                }
              
            }
            else if(hotfireMode == 1 && currentHFState == 4){ // Handle HFS 4

                Serial.println("INPUT 5 TO END TEST");
                Serial.println("5: Close MPVs, open purge, and return to CF mode");
              
                
                if(cmd == '5'){

                  // hotfireMode off
                  // currentHFState = 0
                  // transmit 5 to the mega
                  
                  hotfireMode = 0;
                  currentHFState = 0;
                  // NOTE: Update state vars before going back to CF
                  statevec[0] = 0;
                  statevec[1] = 0;
                  statevec[2] = 0;
                  statevec[3] = 0;
                  statevec[4] = 0;
                  statevec[5] = 0;
                  statevec[6] = 1;
                  statevec[7] = 1;

                  ignFired = 0;
                  engFired = 0;
                  
                  transmitCommand('5');
                }
                else{
                  invalidInput();
                }
              }
            }
            else{
              invalidInput();
            }
          }
          else if(cmd == 'Z'){
            // Check if HFS2
            // If yes-> bleed func 
            // If not say bleed is not avalible in outside of HFS2
            if(currentHFState == 2){
              transmitCommand('Z');
            }
            else{
              Serial.println("LOX tank bleed is not available outside of HFS2");
            }
          }
          else {
            invalidInput();
          }
        }
}
