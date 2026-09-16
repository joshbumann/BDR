#include <SoftwareSerial.h>

// USB serial to laptop
const long USB_BAUD = 115200;

// RS-485 pins on UNO
const uint8_t RS485_RO_PIN = 4;   // RX from MAX485 RO
const uint8_t RS485_RE_PIN = 5;   // RE
const uint8_t RS485_DE_PIN = 6;   // DE
const uint8_t RS485_DI_PIN = 7;   // TX to MAX485 DI

SoftwareSerial RS485Serial(RS485_RO_PIN, RS485_DI_PIN); // rx, tx
const long RS485_BAUD = 9600;

// RS-485 direction helpers
inline void set485Listen() {
  digitalWrite(RS485_RE_PIN, LOW);
  digitalWrite(RS485_DE_PIN, LOW);
}

inline void set485Talk() {
  digitalWrite(RS485_RE_PIN, HIGH);
  digitalWrite(RS485_DE_PIN, HIGH);
}

// Send one command char
void sendRS485Command(char cmd) {
  set485Talk();
  RS485Serial.write(cmd);
  RS485Serial.write('\n');
  RS485Serial.flush();
  delay(2);
  set485Listen();
}

void sendRS485String(const String& msg) {
  set485Talk();
  RS485Serial.print(msg);
  RS485Serial.write('\n');
  RS485Serial.flush();
  delay(2);
  set485Listen();
}

// Help menu
void printHelp() {
  Serial.println();
  Serial.println("Commands:");
  Serial.println("H = Hot fire mode");
  Serial.println("C = Cold flow mode");
  Serial.println("X = Idle / All off");
  Serial.println("1 = Standard Ignition Sequence");
  Serial.println("2 = No Purge Ignition Sequence");
  Serial.println("3 = Initial OX Flood");
  Serial.println("4 = Initial FUEL Flood");
  Serial.println("T = Show current ignition sequence timings");
  Serial.println("SET:Name=value = Change a current timing value");
  Serial.println("S = Start hot fire");
  Serial.println("R = Reset hot fire");
  Serial.println("P = Toggle purge (cold flow)");
  Serial.println("F = Toggle fuel  (cold flow)");
  Serial.println("O = Toggle oxidizer (cold flow)");
  Serial.println("K = Spark ON  (cold flow)");
  Serial.println("L = Spark OFF (cold flow)");
  Serial.println("Q = Status");
  Serial.println("? = Show menu");
  Serial.println();
}

String getInput() {
  String command = Serial.readStringUntil('\n');
  command.trim();
  if (command.length() == 0) {
    return "";
  }
  return command;
}

void setup() {
  Serial.begin(USB_BAUD);

  pinMode(RS485_RE_PIN, OUTPUT);
  pinMode(RS485_DE_PIN, OUTPUT);

  set485Listen();
  RS485Serial.begin(RS485_BAUD);

  Serial.println("UNO RS485 Bridge Ready");
  printHelp();
}

void loop() {
  if (Serial.available() > 0) {
    String cmd = getInput();

    if (cmd.length() == 0) {
      return;
    }

    String upper = cmd;
    upper.toUpperCase();

    if (upper == "?") {
      printHelp();
      return;
    }

    if (upper == "T") {
      sendRS485Command('T');
      Serial.println("Sent: T");
      return;
    }

    if (cmd.startsWith("SET:")) {
      sendRS485String(cmd);
      Serial.print("Sent: ");
      Serial.println(cmd);
      return;
    }

    if (cmd.length() == 1) {
      char c = cmd.charAt(0);
      if (c == 'H' || c == 'C' || c == 'X' ||
          c == '1' || c == '2' || c == '3' || c == '4' ||
          c == 'S' || c == 'R' ||
          c == 'P' || c == 'F' || c == 'O' ||
          c == 'K' || c == 'L' ||
          c == 'Q') {
        sendRS485Command(c);
        Serial.print("Sent: ");
        Serial.println(c);
        return;
      }
    }

    Serial.println("Invalid command.");
  }

  while (RS485Serial.available() > 0) {
    String reply = RS485Serial.readStringUntil('\n');
    reply.trim();

    if (reply.length() > 0) {
      Serial.print("Mega: ");
      Serial.println(reply);
    }
  }
}