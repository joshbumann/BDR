#include <SoftwareSerial.h>

#define OPEN true
#define CLOSED false

enum State {
  IDLE,
  PRE_PURGE,
  SPARK,
  OX_FLOOD,
  SECOND_OX_FLOOD,
  FUEL_FLOOD,
  SECOND_FUEL_FLOOD,
  MAIN_BURN,
  POST_PURGE,
  DONE
};
State Current_state = IDLE;
enum Mode {
  IDLE_MODE,
  COLD_FLOW_MODE,
  HOT_FIRE_MODE
};
Mode Current_mode = IDLE_MODE;

const int RS485_Baud = 9600;

// RS-485 direction pins on Mega
const uint8_t RS485_RO_PIN = 13;
const uint8_t RS485_DI_PIN = 10;
const uint8_t RS485_DE_PIN = 11;
const uint8_t RS485_RE_PIN = 12;
SoftwareSerial RS485Serial(RS485_RO_PIN, RS485_DI_PIN);

// Spark Plug Frequency and Half Period Of Wave
const float Frequency = 60.0;
const unsigned long Half_Period = (unsigned long)(1000.0 / (Frequency * 2.0));

// Pin Assignment
const int Purge_Pin = A1;
const int Spark_Pin = A2;
const int Fuel_Pin = A3;
const int Oxidizer_Pin = A4;

unsigned long State_start_time = 0;

// State Time Values (milliseconds)
unsigned long Pre_Purge_time = 1500;
unsigned long Main_Burn_time = 3000;
unsigned long Post_Purge_time = 2000;
unsigned long Spark_Lead_time = 500;
unsigned long OX_Flood_time = 1000;
unsigned long SECOND_OX_Flood_time = 2500;
unsigned long FUEL_Flood_time = 1000;
unsigned long SECOND_FUEL_Flood_time = 2500;
// State Command Initialization
bool startCommand = false;
bool resetCommand = false;
int ignitionSequence = 1;  // default ignition sequence

// Cold Flow State Conditions
bool Cold_Flow_Purge = CLOSED;
bool Cold_Flow_Fuel = CLOSED;
bool Cold_Flow_Ox = CLOSED;
bool Cold_Flow_Spark = CLOSED;

// Spark Pulse Logic
bool Spark_state = false;
unsigned long Last_Spark_Toggle_time = 0;

inline void set485Listen() {
  digitalWrite(RS485_RE_PIN, LOW);
  digitalWrite(RS485_DE_PIN, LOW);
}

inline void set485Talk() {
  digitalWrite(RS485_RE_PIN, HIGH);
  digitalWrite(RS485_DE_PIN, HIGH);
}

void transmitMessage(const char* msg) {
  set485Talk();
  RS485Serial.println(msg);
  RS485Serial.flush();
  delay(2);
  set485Listen();
}

void set_State(bool purge, bool fuel, bool ox) {
  digitalWrite(Purge_Pin, purge ? HIGH : LOW);
  digitalWrite(Fuel_Pin, fuel ? HIGH : LOW);
  digitalWrite(Oxidizer_Pin, ox ? HIGH : LOW);
}

void allOutputsOff() {
  digitalWrite(Purge_Pin, LOW);
  digitalWrite(Fuel_Pin, LOW);
  digitalWrite(Oxidizer_Pin, LOW);
  digitalWrite(Spark_Pin, LOW);
  Spark_state = false;
}

void enterIdleMode() {
  Current_mode = IDLE_MODE;
  Current_state = IDLE;
  startCommand = false;
  resetCommand = false;

  Cold_Flow_Purge = CLOSED;
  Cold_Flow_Fuel = CLOSED;
  Cold_Flow_Ox = CLOSED;
  Cold_Flow_Spark = CLOSED;

  allOutputsOff();
}

void enterColdFlowMode() {
  Current_mode = COLD_FLOW_MODE;
  Current_state = IDLE;
  startCommand = false;
  resetCommand = false;

  Cold_Flow_Purge = CLOSED;
  Cold_Flow_Fuel = CLOSED;
  Cold_Flow_Ox = CLOSED;
  Cold_Flow_Spark = CLOSED;

  allOutputsOff();
}

void enterHotFireMode() {
  Current_mode = HOT_FIRE_MODE;
  Current_state = IDLE;
  startCommand = false;
  resetCommand = false;

  Cold_Flow_Purge = CLOSED;
  Cold_Flow_Fuel = CLOSED;
  Cold_Flow_Ox = CLOSED;
  Cold_Flow_Spark = CLOSED;

  allOutputsOff();
}

void Enter_state(State New_state) {
  State_start_time = millis();
  Current_state = New_state;

  switch (Current_state) {
    case IDLE:
      set_state(CLOSED, CLOSED, CLOSED);
      break;
    case PRE_PURGE:
      set_state(OPEN, CLOSED, CLOSED);
      break;
    case SPARK:
      Last_Spark_Toggle_time = millis();
      break;
    case OX_FLOOD:
      set_state(OPEN, CLOSED, OPEN);
      break;
    case FUEL_FLOOD:
      set_state(OPEN, OPEN, CLOSED);
      break;
    case SECOND_OX_FLOOD:
      set_state(OPEN, CLOSED, OPEN);
      break;
    case MAIN_BURN:
      set_state(OPEN, OPEN, OPEN);
      break;
    case SECOND_FUEL_FLOOD:
      set_state(OPEN, OPEN, CLOSED);
      break;
    case POST_PURGE:
      set_state(OPEN, CLOSED, CLOSED);
      break;
    case DONE:
      set_state(CLOSED, CLOSED, CLOSED);
      break;
  }
}

void hot_fire_ignition_sequence(int ignitionSequence) {
  unsigned long elapsed = millis() - State_start_time;
  if (resetCommand) {
    resetCommand = false;
    Enter_state(IDLE);
  }
  switch (ignitionSequence) {
    case 1: // Standard Ignition Sequence
      switch (Current_state) {
        case IDLE:
          if (startCommand) {
            startCommand = false;
            Enter_state(PRE_PURGE);
          }
          break;
        case PRE_PURGE:
          if (elapsed >= Pre_Purge_time) {
            Enter_state(SPARK);
          }
          break;
        case SPARK:
          if (elapsed >= Spark_Lead_time) {
            Enter_state(MAIN_BURN);
          }
          break;
        case MAIN_BURN:
          if (elapsed >= Main_Burn_time) {
            Enter_state(POST_PURGE);
          }
          break;
        case POST_PURGE:
          if (elapsed >= Post_Purge_time) {
            Enter_state(DONE);
          }
          break;
        case DONE:
          break;
      }
    case 2: // No Purge Ignition Sequence
      switch (Current_state) {
        case IDLE:
          if (startCommand) {
            startCommand = false;
            Enter_state(SPARK);
          }
          break;
        case SPARK:
          if (elapsed >= Spark_Lead_time) {
            Enter_state(MAIN_BURN);
          }
          break;
        case MAIN_BURN:
          if (elapsed >= Main_Burn_time) {
            Enter_state(POST_PURGE);
          }
          break;
        case POST_PURGE:
          if (elapsed >= Post_Purge_time) {
            Enter_state(DONE);
          }
          break;
        case DONE:
          break;
      }
    case 3: // Initial OX Flood
      switch (Current_state) {
        case IDLE:
          if (startCommand) {
            startCommand = false;
            Enter_state(OX_FLOOD);
          }
          break;
        case OX_FLOOD:
          if (elapsed >= OX_Flood_time) {
            Enter_state(SPARK);
          }
          break;
        case SPARK:
          if (elapsed >= Spark_Lead_time) {
            Enter_state(MAIN_BURN);
          }
          break;
        case MAIN_BURN:
          if (elapsed >= Main_Burn_time) {
            Enter_state(OX_FLOOD);
          }
          break;
        case SECOND_OX_FLOOD:
          if (elapsed >= SECOND_OX_Flood_time) {
            Enter_state(POST_PURGE);
          }
          break;  
        case POST_PURGE:
          if (elapsed >= Post_Purge_time) {
            Enter_state(DONE);
          }
          break;
        case DONE:
          break;
      }
    case 4: // Initial FUEL Flood
      switch (Current_state) {
        case IDLE:
          if (startCommand) {
            startCommand = false;
            Enter_state(FUEL_FLOOD);
          }
          break;
        case FUEL_FLOOD:
          if (elapsed >= FUEL_Flood_time) {
            Enter_state(SPARK);
          }
          break;
        case SPARK:
          if (elapsed >= Spark_Lead_time) {
            Enter_state(MAIN_BURN);
          }
          break;
        case MAIN_BURN:
          if (elapsed >= Main_Burn_time) {
            Enter_state(FUEL_FLOOD);
          }
          break;
        case SECOND_FUEL_FLOOD:
          if (elapsed >= SECOND_FUEL_Flood_time) {
            Enter_state(POST_PURGE);
          }
          break;  
        case POST_PURGE:
          if (elapsed >= Post_Purge_time) {
            Enter_state(DONE);
          }
          break;
        case DONE:
          break;
      }
    }
}

void applyColdFlowOutputs() {
  if (Current_mode != COLD_FLOW_MODE) return;

  digitalWrite(Purge_Pin, Cold_Flow_Purge ? HIGH : LOW);
  digitalWrite(Fuel_Pin, Cold_Flow_Fuel ? HIGH : LOW);
  digitalWrite(Oxidizer_Pin, Cold_Flow_Ox ? HIGH : LOW);
}

void Update_Spark_Pulse() {
  bool Spark_active = false;

  if (Current_mode == HOT_FIRE_MODE) {
    Spark_active = (Current_state == SPARK) || (Current_state == MAIN_BURN) || (Current_state == SECOND_FUEL_FLOOD);
  } else if (Current_mode == COLD_FLOW_MODE) {
    Spark_active = Cold_Flow_Spark;
  }

  if (!Spark_active) {
    Spark_state = false;
    digitalWrite(Spark_Pin, LOW);
    return;
  }

  unsigned long Current_time_Spark = millis();

  if (Current_time_Spark - Last_Spark_Toggle_time >= Half_Period) {
    Last_Spark_Toggle_time = Current_time_Spark;
    Spark_state = !Spark_state;
    digitalWrite(Spark_Pin, Spark_state ? HIGH : LOW);
  }
}

const char* modeToString() {
  switch (Current_mode) {
    case IDLE_MODE: return "IDLE";
    case COLD_FLOW_MODE: return "COLD_FLOW";
    case HOT_FIRE_MODE: return "HOT_FIRE";
  }
  return "UNKNOWN";
}

const char* stateToString() {
  switch (Current_state) {
    case IDLE: return "IDLE";
    case PRE_PURGE: return "PRE_PURGE";
    case SPARK: return "SPARK";
    case OX_FLOOD: return "OX_FLOOD";
    case FUEL_FLOOD: return "FUEL_FLOOD";
    case MAIN_BURN: return "MAIN_BURN";
    case SECOND_FUEL_FLOOD: return "SECOND_FUEL_FLOOD";
    case POST_PURGE: return "POST_PURGE";
    case DONE: return "DONE";
  }
  return "UNKNOWN";
}

void sendStatus() {
  transmitMessage(modeToString());
  transmitMessage(stateToString());
  transmitMessage(Cold_Flow_Purge ? "PURGE_ON" : "PURGE_OFF");
  transmitMessage(Cold_Flow_Fuel ? "FUEL_ON" : "FUEL_OFF");
  transmitMessage(Cold_Flow_Ox ? "OX_ON" : "OX_OFF");
  transmitMessage(Cold_Flow_Spark ? "SPARK_ON" : "SPARK_OFF");
}

void handleCommand(char cmd) {
  switch (cmd) {
    case '1':
      ignitionSequence = 1;
      transmitMessage("ACK:1");
      break;
    case '2':
      ignitionSequence = 2;
      transmitMessage("ACK:2");
      break;
    case '3':
      ignitionSequence = 3;
      transmitMessage("ACK:3");
      break;
    case '4':
      ignitionSequence = 4;
      transmitMessage("ACK:4");
      break;
    case 'H':
      enterHotFireMode();
      transmitMessage("ACK:H");
      break;
    case 'C':
      enterColdFlowMode();
      transmitMessage("ACK:C");
      break;
    case 'X':
      enterIdleMode();
      transmitMessage("ACK:X");
      break;
    case 'S':
      if (Current_mode == HOT_FIRE_MODE && Current_state == IDLE) {
        startCommand = true;
        transmitMessage("ACK:S");
      } else {
        transmitMessage("ERR");
      }
      break;
    case 'R':
      if (Current_mode == HOT_FIRE_MODE) {
        resetCommand = true;
        transmitMessage("ACK:R");
      } else {
        transmitMessage("ERR");
      }
      break;
    case 'P':
      if (Current_mode == COLD_FLOW_MODE) {
        Cold_Flow_Purge = !Cold_Flow_Purge;
        applyColdFlowOutputs();
        transmitMessage("ACK:P");
      } else {
        transmitMessage("ERR");
      }
      break;
    case 'F':
      if (Current_mode == COLD_FLOW_MODE) {
        Cold_Flow_Fuel = !Cold_Flow_Fuel;
        applyColdFlowOutputs();
        transmitMessage("ACK:F");
      } else {
        transmitMessage("ERR");
      }
      break;
    case 'O':
      if (Current_mode == COLD_FLOW_MODE) {
        Cold_Flow_Ox = !Cold_Flow_Ox;
        applyColdFlowOutputs();
        transmitMessage("ACK:O");
      } else {
        transmitMessage("ERR");
      }
      break;
    case 'K':
      if (Current_mode == COLD_FLOW_MODE) {
        Cold_Flow_Spark = true;
        Last_Spark_Toggle_time = millis();
        Spark_state = false;
        transmitMessage("ACK:K");
      } else {
        transmitMessage("ERR");
      }
      break;
    case 'L':
      if (Current_mode == COLD_FLOW_MODE) {
        Cold_Flow_Spark = false;
        transmitMessage("ACK:L");
      } else {
        transmitMessage("ERR");
      }
      break;
    case 'Q':
      transmitMessage("ACK:Q");
      sendStatus();
      break;
    default:
      transmitMessage("ERR");
      break;
  }
}

void readRS485Commands() {
  while (RS485Serial.available() > 0) {
    int raw = RS485Serial.read();   // returns -1 to 255
    if (raw == -1) break;           // safety check
    char c = (char)raw;             // convert byte to char
    if (c == '\n' || c == '\r') continue;
    handleCommand(c);
  }
}

void setup() {
  RS485Serial.begin(RS485_Baud);

  pinMode(RS485_RE_PIN, OUTPUT);
  pinMode(RS485_DE_PIN, OUTPUT);

  pinMode(Purge_Pin, OUTPUT);
  pinMode(Spark_Pin, OUTPUT);
  pinMode(Fuel_Pin, OUTPUT);
  pinMode(Oxidizer_Pin, OUTPUT);

  set485Listen();
  enterIdleMode();
  int ignitionSequence = 1; // Default ignition sequence
}

void loop() {
  readRS485Commands();

  if (Current_mode == HOT_FIRE_MODE) {
    unsigned long elapsed = millis() - State_start_time;

    if (resetCommand) {
      resetCommand = false;
      Enter_state(IDLE);
    }

    hot_fire_ignition_sequence(ignitionSequence);
  }

  if (Current_mode == COLD_FLOW_MODE) {
    applyColdFlowOutputs();
  }

  Update_Spark_Pulse();
}