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
State Current_State = IDLE;
enum Mode {
  IDLE_MODE,
  COLD_FLOW_MODE,
  HOT_FIRE_MODE
};
Mode Current_Mode = IDLE_MODE;

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
const int Purge_Pin = 28;
const int Spark_Pin = A2;
const int Fuel_Pin = 29;
const int Oxidizer_Pin = 30;

unsigned long State_Start_Time = 0;

// State Time Values (milliseconds)
unsigned long Pre_Purge_Time = 1500;
unsigned long Main_Burn_Time = 3000;
unsigned long Post_Purge_Time = 2000;
unsigned long Spark_Lead_Time = 500;
unsigned long OX_Flood_Time = 1000;
unsigned long SECOND_OX_Flood_Time = 2500;
unsigned long FUEL_Flood_Time = 1000;
unsigned long SECOND_FUEL_Flood_Time = 2500;

struct TimingEntry {
  const char* name;
  unsigned long* value;
};

TimingEntry Sequence_1[] = {
  {"Pre_Purge_Time", &Pre_Purge_Time},
  {"Spark_Lead_Time", &Spark_Lead_Time},
  {"Main_Burn_Time", &Main_Burn_Time},
  {"Post_Purge_Time", &Post_Purge_Time}
};

TimingEntry Sequence_2[] = {
  {"Spark_Lead_Time", &Spark_Lead_Time},
  {"Main_Burn_Time", &Main_Burn_Time},
  {"Post_Purge_Time", &Post_Purge_Time}
};

TimingEntry Sequence_3[] = {
  {"OX_Flood_Time", &OX_Flood_Time},
  {"Spark_Lead_Time", &Spark_Lead_Time},
  {"Main_Burn_Time", &Main_Burn_Time},
  {"SECOND_OX_Flood_Time", &SECOND_OX_Flood_Time},
  {"Post_Purge_Time", &Post_Purge_Time}
};

TimingEntry Sequence_4[] = {
  {"FUEL_Flood_Time", &FUEL_Flood_Time},
  {"Spark_Lead_Time", &Spark_Lead_Time},
  {"Main_Burn_Time", &Main_Burn_Time},
  {"SECOND_FUEL_Flood_Time", &SECOND_FUEL_Flood_Time},
  {"Post_Purge_Time", &Post_Purge_Time}
};

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

  Cold_Flow_Purge = CLOSED;
  Cold_Flow_Fuel = CLOSED;
  Cold_Flow_Ox = CLOSED;
  Cold_Flow_Spark = CLOSED;

  allOutputsOff();
}

void enterColdFlowMode() {
  Current_Mode = COLD_FLOW_MODE;
  Current_State = IDLE;
  startCommand = false;
  resetCommand = false;

  Cold_Flow_Purge = CLOSED;
  Cold_Flow_Fuel = CLOSED;
  Cold_Flow_Ox = CLOSED;
  Cold_Flow_Spark = CLOSED;

  allOutputsOff();
}

void enterHotFireMode() {
  Current_Mode = HOT_FIRE_MODE;
  Current_State = IDLE;
  startCommand = false;
  resetCommand = false;

  Cold_Flow_Purge = CLOSED;
  Cold_Flow_Fuel = CLOSED;
  Cold_Flow_Ox = CLOSED;
  Cold_Flow_Spark = CLOSED;

  allOutputsOff();
}

void Enter_State(State New_State) {
  State_Start_Time = millis();
  Current_State = New_State;

  switch (Current_State) {
    case IDLE:
      set_State(CLOSED, CLOSED, CLOSED);
      break;
    case PRE_PURGE:
      set_State(OPEN, CLOSED, CLOSED);
      break;
    case SPARK:
      Last_Spark_Toggle_Time = millis();
      break;
    case OX_FLOOD:
      set_State(OPEN, CLOSED, OPEN);
      break;
    case FUEL_FLOOD:
      set_State(OPEN, OPEN, CLOSED);
      break;
    case SECOND_OX_FLOOD:
      set_State(OPEN, CLOSED, OPEN);
      break;
    case MAIN_BURN:
      set_State(OPEN, OPEN, OPEN);
      break;
    case SECOND_FUEL_FLOOD:
      set_State(OPEN, OPEN, CLOSED);
      break;
    case POST_PURGE:
      set_State(OPEN, CLOSED, CLOSED);
      break;
    case DONE:
      set_State(CLOSED, CLOSED, CLOSED);
      break;
  }
}

void hot_fire_ignition_sequence(int ignitionSequence) {
  unsigned long elapsed = millis() - State_Start_Time;
  if (resetCommand) {
    resetCommand = false;
    Enter_State(IDLE);
  }
  switch (ignitionSequence) {
    case 1: // Standard Ignition Sequence
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
      break;
    case 2: // No Purge Ignition Sequence
      switch (Current_State) {
        case IDLE:
          if (startCommand) {
            startCommand = false;
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
      break;
    case 3: // Initial OX Flood
      switch (Current_State) {
        case IDLE:
          if (startCommand) {
            startCommand = false;
            Enter_State(OX_FLOOD);
          }
          break;
        case OX_FLOOD:
          if (elapsed >= OX_Flood_Time) {
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
            Enter_State(SECOND_OX_FLOOD);
          }
          break;
        case SECOND_OX_FLOOD:
          if (elapsed >= SECOND_OX_Flood_Time) {
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
      break;
    case 4: // Initial FUEL Flood
      switch (Current_State) {
        case IDLE:
          if (startCommand) {
            startCommand = false;
            Enter_State(FUEL_FLOOD);
          }
          break;
        case FUEL_FLOOD:
          if (elapsed >= FUEL_Flood_Time) {
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
            Enter_State(SECOND_FUEL_FLOOD);
          }
          break;
        case SECOND_FUEL_FLOOD:
          if (elapsed >= SECOND_FUEL_Flood_Time) {
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
    Spark_Active = (Current_State == SPARK) || (Current_State == MAIN_BURN) || (Current_State == SECOND_FUEL_FLOOD);
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
    case OX_FLOOD: return "OX_FLOOD";
    case FUEL_FLOOD: return "FUEL_FLOOD";
    case MAIN_BURN: return "MAIN_BURN";
    case SECOND_FUEL_FLOOD: return "SECOND_FUEL_FLOOD";
    case POST_PURGE: return "POST_PURGE";
    case DONE: return "DONE";
  }
  return "UNKNOWN";
}

const char* getSequenceName(int sequence) {
  switch (sequence) {
    case 1: return "Standard Ignition Sequence";
    case 2: return "No Purge Ignition Sequence";
    case 3: return "Initial OX Flood";
    case 4: return "Initial FUEL Flood";
    default: return "Unknown Ignition Sequence";
  }
}

TimingEntry* getSequenceEntries(int sequence, int& count) {
  switch (sequence) {
    case 1:
      count = sizeof(Sequence_1) / sizeof(Sequence_1[0]);
      return Sequence_1;
    case 2:
      count = sizeof(Sequence_2) / sizeof(Sequence_2[0]);
      return Sequence_2;
    case 3:
      count = sizeof(Sequence_3) / sizeof(Sequence_3[0]);
      return Sequence_3;
    case 4:
      count = sizeof(Sequence_4) / sizeof(Sequence_4[0]);
      return Sequence_4;
    default:
      static TimingEntry defaultSeq[] = {
        {"Pre_Purge_Time", &Pre_Purge_Time}
      };
      count = 1;
      return defaultSeq;
  }
}

bool setTimingValueByName(const String& name, unsigned long newValue) {
  int count = 0;
  TimingEntry* entries = getSequenceEntries(ignitionSequence, count);

  for (int i = 0; i < count; ++i) {
    if (name.equalsIgnoreCase(entries[i].name)) {
      *entries[i].value = newValue;
      return true;
    }
  }

  return false;
}

void sendTimingTable() {
  int count = 0;
  TimingEntry* entries = getSequenceEntries(ignitionSequence, count);

  String header = "SEQ:" + String(ignitionSequence) + ":" + getSequenceName(ignitionSequence);
  transmitMessage(header.c_str());

  for (int i = 0; i < count; ++i) {
    String line = String(entries[i].name) + "=" + String(*entries[i].value);
    transmitMessage(line.c_str());
  }
}

void handleCommandString(const String& packet) {
  String cmd = packet;
  cmd.trim();

  if (cmd.equalsIgnoreCase("T")) {
    sendTimingTable();
    return;
  }

  if (cmd.startsWith("SET:")) {
    String payload = cmd.substring(4);
    int equalsIndex = payload.indexOf('=');

    if (equalsIndex < 0) {
      transmitMessage("ERR:FORMAT");
      return;
    }

    String name = payload.substring(0, equalsIndex);
    String valueText = payload.substring(equalsIndex + 1);
    name.trim();
    valueText.trim();

    if (name.length() == 0 || valueText.length() == 0) {
      transmitMessage("ERR:FORMAT");
      return;
    }

    unsigned long newValue = valueText.toInt();
    if (setTimingValueByName(name, newValue)) {
      String ack = "SET_OK:" + name + "=" + String(newValue);
      transmitMessage(ack.c_str());
    } else {
      transmitMessage("ERR:NAME");
    }
    return;
  }

  transmitMessage("ERR");
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
    case 'T':
      sendTimingTable();
      break;
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
    String packet = RS485Serial.readStringUntil('\n');
    packet.trim();

    if (packet.length() == 0) {
      continue;
    }

    if (packet.length() == 1) {
      handleCommand(packet.charAt(0));
    } else {
      handleCommandString(packet);
    }
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

    hot_fire_ignition_sequence(ignitionSequence);
  }

  if (Current_Mode == COLD_FLOW_MODE) {
    applyColdFlowOutputs();
  }

  Update_Spark_Pulse();
}