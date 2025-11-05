#include <SoftwareSerial.h>

// RS-485 pins on UNO
const uint8_t RS485_RO_PIN = 4;  // RX from MAX485 RO
const uint8_t RS485_RE_PIN = 5;  // RE (LOW = listen, HIGH = talk)
const uint8_t RS485_DE_PIN = 6;  // DE (LOW = listen, HIGH = talk)
const uint8_t RS485_DI_PIN = 7;  // TX to MAX485 DI

SoftwareSerial RS485Serial(RS485_RO_PIN, RS485_DI_PIN); // rx, tx

bool statevec[8] = {0,0,0,0,0,0,0,0};

inline void set485Listen() {   // Receive mode: RE=LOW, DE=LOW
  digitalWrite(RS485_RE_PIN, LOW);
  digitalWrite(RS485_DE_PIN, LOW);
}
inline void set485Talk() {     // Transmit mode: RE=HIGH, DE=HIGH
  digitalWrite(RS485_RE_PIN, HIGH);
  digitalWrite(RS485_DE_PIN, HIGH);
}

void stateCheck(){
  for(int i = 0; i<8; i++){
     String stateStr = "";
      switch(i){
      case 0:
        stateStr += "Fuel N2 Valve:    ";   

      break;
      case 1:
        stateStr += "LOX N2 Valve:     ";

      break;
      case 2:
        stateStr += "Main Fuel Valve:  ";

      break;
      case 3:
        stateStr += "Main O2 Valve:    ";
        
      break;
      case 4:
        stateStr += "Fuel Vent Valve:  ";
        
      break;
      case 5:
        stateStr += "O2 Vent Valve:    ";

      break;
      case 6:
        stateStr += "Fuel Purge Valve: ";

      break;
      case 7:
        stateStr += "O2 Purge Valve:   ";

      break;

      if(statevec[i] == 0){
        stateStr += "CLOSED";
      }
      else if(statevec[i] == 1){
        stateStr += "OPEN";
      }
      

      Serial.println(stateStr);
    
    }
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



void loop() {

  if (Serial.available()) {

    
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
    Serial.println("N) Check Valve States");
    
    
    Serial.println("Enter the option of the valve you want to toggle: ");
    String command = Serial.readStringUntil('\n'); // Read until a newline character
    command.trim(); // Remove any whitespace
    
    char cmd = 0;
   
        for (uint16_t i = 0; i < command.length(); ++i) {
          char c = command[i];
          if (c >= 'A' && c <= 'M') { cmd = c; break; }
        }
        
    
        if (cmd) {
          if(cmd >= 'A' && cmd <= 'M'){
            set485Talk();
            RS485Serial.write(cmd);
            RS485Serial.write('\n');
            RS485Serial.flush();   // ensure bytes left TX buffer
            delay(2);              // small guard at 9600 bps
            set485Listen();

            
            switch(cmd){   // Keep track of states
              case 'A':
                statevec[1] = !statevec[1];
              break;
              case 'B':
                statevec[2] = !statevec[2];
              break;
              case 'C':
                statevec[3] = !statevec[3];
              break;
              case 'D':
                statevec[4] = !statevec[4];
              break;
              case 'E':
                statevec[5] = !statevec[5];
              break;
              case 'F':
                statevec[6] = !statevec[6];
              break;
              case 'G':
                statevec[7] = !statevec[7];
              break;
              case 'H':
                statevec[8] = !statevec[8];
              break;
              case 'I':
                statevec[3] = !statevec[3];
                statevec[4] = !statevec[4];
              break;
              case 'J':
                statevec[1] = !statevec[2];
                statevec[1] = !statevec[2];
              break;
              case 'K':
                statevec[5] = !statevec[5];
                statevec[6] = !statevec[6];
              break;
              case 'L':
                statevec[7] = !statevec[7];
                statevec[8] = !statevec[8];
              break;
              case 'M':
                for(int i = 0; i < 8; i++){
                  statevec[i] = 0;
                }
              break;
              }
            
          }
          else if(cmd == 'N'){
            stateCheck();
          }
        }
        else {
          Serial.println("Ignored. Send a single digit 1..8.");
        }
  }
}
