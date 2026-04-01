#include <SoftwareSerial.h>
const int RS485_Baud = 9600;

// RS-485 direction pins on Mega
const uint8_t RS485_RO_PIN = 13;
const uint8_t RS485_DI_PIN = 10;
SoftwareSerial RS485Serial(RS485_RO_PIN, RS485_DI_PIN);

const uint8_t RS485_DE_PIN = 11;
const uint8_t RS485_RE_PIN = 12;



enum Mode {
  IDLE_MODE,
  COLD_FLOW_MODE,
  HOT_FIRE_MODE
};

Mode Current_Mode = IDLE_MODE;

enum State {
  IDLE,
  PRE_PURGE,
  SPARK,
  MAIN_BURN,
  POST_PURGE,
  DONE
};

State Current_State = IDLE;
unsigned long State_Start_Time = 0;

// State Time Values (milliseconds)
const unsigned long Pre_Purge_Time = 1500;
const unsigned long Main_Burn_Time = 3000;
const unsigned long Post_Purge_Time = 2000;
const unsigned long Spark_Lead_Time = 500;

// Spark Plug Frequency and Half Period Of Wave
const float Frequency = 60.0;
const unsigned long Half_Period = (unsigned long)(1000.0 / (Frequency * 2.0));

// Pin Assignment
const int Purge_Pin = A1;
const int Spark_Pin = A2;
const int Fuel_Pin = A3;
const int Oxidizer_Pin = A4;

// State Command Initialization
bool startCommand = false;
bool resetCommand = false;

// Cold Flow State Conditions
bool Cold_Flow_Purge = false;
bool Cold_Flow_Fuel = false;
bool Cold_Flow_Ox = false;
bool Cold_Flow_Spark = false;

// Spark Pulse Logic
bool Spark_State = false;
unsigned long Last_Spark_Toggle_Time = 0;

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
  Spark_State = false;
}

void enterIdleMode() {
  Current_Mode = IDLE_MODE;
  Current_State = IDLE;
  startCommand = false;
  resetCommand = false;

  Cold_Flow_Purge = false;
  Cold_Flow_Fuel = false;
  Cold_Flow_Ox = false;
  Cold_Flow_Spark = false;

  allOutputsOff();
}

void enterColdFlowMode() {
  Current_Mode = COLD_FLOW_MODE;
  Current_State = IDLE;
  startCommand = false;
  resetCommand = false;

  Cold_Flow_Purge = false;
  Cold_Flow_Fuel = false;
  Cold_Flow_Ox = false;
  Cold_Flow_Spark = false;

  allOutputsOff();
}

void enterHotFireMode() {
  Current_Mode = HOT_FIRE_MODE;
  Current_State = IDLE;
  startCommand = false;
  resetCommand = false;

  Cold_Flow_Purge = false;
  Cold_Flow_Fuel = false;
  Cold_Flow_Ox = false;
  Cold_Flow_Spark = false;

  allOutputsOff();
}

void Enter_State(State New_State) {
  State_Start_Time = millis();
  Current_State = New_State;

  switch (Current_State) {
    case IDLE:
      set_State(false, false, false);
      break;
    case PRE_PURGE:
      set_State(true, false, false);
      break;
    case SPARK:
      set_State(true, false, false);
      Last_Spark_Toggle_Time = millis();
      break;
    case MAIN_BURN:
      set_State(true, true, true);
      break;
    case POST_PURGE:
      set_State(true, false, false);
      break;
    case DONE:
      set_State(false, false, false);
      break;
  }
}

void applyColdFlowOutputs() {
  if (Current_Mode != COLD_FLOW_MODE) return;

  digitalWrite(Purge_Pin, Cold_Flow_Purge ? HIGH : LOW);
  digitalWrite(Fuel_Pin, Cold_Flow_Fuel ? HIGH : LOW);
  digitalWrite(Oxidizer_Pin, Cold_Flow_Ox ? HIGH : LOW);
}

void Update_Spark_Pulse() {
  bool Spark_Active = false;

  if (Current_Mode == HOT_FIRE_MODE) {
    Spark_Active = (Current_State == SPARK) || (Current_State == MAIN_BURN);
  } else if (Current_Mode == COLD_FLOW_MODE) {
    Spark_Active = Cold_Flow_Spark;
  }

  if (!Spark_Active) {
    Spark_State = false;
    digitalWrite(Spark_Pin, LOW);
    return;
  }

  unsigned long Current_Time_Spark = millis();

  if (Current_Time_Spark - Last_Spark_Toggle_Time >= Half_Period) {
    Last_Spark_Toggle_Time = Current_Time_Spark;
    Spark_State = !Spark_State;
    digitalWrite(Spark_Pin, Spark_State ? HIGH : LOW);
  }
}

const char* modeToString() {
  switch (Current_Mode) {
    case IDLE_MODE: return "IDLE";
    case COLD_FLOW_MODE: return "COLD_FLOW";
    case HOT_FIRE_MODE: return "HOT_FIRE";
  }
  return "UNKNOWN";
}

const char* stateToString() {
  switch (Current_State) {
    case IDLE: return "IDLE";
    case PRE_PURGE: return "PRE_PURGE";
    case SPARK: return "SPARK";
    case MAIN_BURN: return "MAIN_BURN";
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
      if (Current_Mode == HOT_FIRE_MODE && Current_State == IDLE) {
        startCommand = true;
        transmitMessage("ACK:S");
      } else {
        transmitMessage("ERR");
      }
      break;
    case 'R':
      if (Current_Mode == HOT_FIRE_MODE) {
        resetCommand = true;
        transmitMessage("ACK:R");
      } else {
        transmitMessage("ERR");
      }
      break;
    case 'P':
      if (Current_Mode == COLD_FLOW_MODE) {
        Cold_Flow_Purge = !Cold_Flow_Purge;
        applyColdFlowOutputs();
        transmitMessage("ACK:P");
      } else {
        transmitMessage("ERR");
      }
      break;
    case 'F':
      if (Current_Mode == COLD_FLOW_MODE) {
        Cold_Flow_Fuel = !Cold_Flow_Fuel;
        applyColdFlowOutputs();
        transmitMessage("ACK:F");
      } else {
        transmitMessage("ERR");
      }
      break;
    case 'O':
      if (Current_Mode == COLD_FLOW_MODE) {
        Cold_Flow_Ox = !Cold_Flow_Ox;
        applyColdFlowOutputs();
        transmitMessage("ACK:O");
      } else {
        transmitMessage("ERR");
      }
      break;
    case 'K':
      if (Current_Mode == COLD_FLOW_MODE) {
        Cold_Flow_Spark = true;
        Last_Spark_Toggle_Time = millis();
        Spark_State = false;
        transmitMessage("ACK:K");
      } else {
        transmitMessage("ERR");
      }
      break;
    case 'L':
      if (Current_Mode == COLD_FLOW_MODE) {
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
    char c = RS485Serial.read();
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
}

void loop() {
  readRS485Commands();

  if (Current_Mode == HOT_FIRE_MODE) {
    unsigned long elapsed = millis() - State_Start_Time;

    if (resetCommand) {
      resetCommand = false;
      Enter_State(IDLE);
    }

    switch (Current_State) {
      case IDLE:
        if (startCommand) {
          startCommand = false;
          Enter_State(PRE_PURGE);
        }
        break;
      case PRE_PURGE:
        if (elapsed >= Pre_Purge_Time) {
          Enter_State(SPARK);
        }
        break;
      case SPARK:
        if (elapsed >= Spark_Lead_Time) {
          Enter_State(MAIN_BURN);
        }
        break;
      case MAIN_BURN:
        if (elapsed >= Main_Burn_Time) {
          Enter_State(POST_PURGE);
        }
        break;
      case POST_PURGE:
        if (elapsed >= Post_Purge_Time) {
          Enter_State(DONE);
        }
        break;
      case DONE:
        break;
    }
  }

  if (Current_Mode == COLD_FLOW_MODE) {
    applyColdFlowOutputs();
  }

  Update_Spark_Pulse();
}