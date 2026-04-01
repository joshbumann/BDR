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

// Help menu
void printHelp() {
  Serial.println();
  Serial.println("Commands:");
  Serial.println("H = Hot fire mode");
  Serial.println("C = Cold flow mode");
  Serial.println("X = Idle / All off");
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

// Read laptop input and extract first valid command
char getInput() {
  String command = Serial.readStringUntil('\n');
  command.trim();
  command.toUpperCase();

  for (uint16_t i = 0; i < command.length(); ++i) {
    char c = command[i];

    if (c == 'H' || c == 'C' || c == 'X' ||
        c == 'S' || c == 'R' ||
        c == 'P' || c == 'F' || c == 'O' ||
        c == 'K' || c == 'L' ||
        c == 'Q' || c == '?') {
      return c;
    }
  }

  return 0;
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
  // Laptop -> Mega
  if (Serial.available() > 0) {
    char cmd = getInput();

    if (cmd == '?') {
      printHelp();
    }
    else if (cmd != 0) {
      sendRS485Command(cmd);
      Serial.print("Sent: ");
      Serial.println(cmd);
    }
    else {
      Serial.println("Invalid command.");
    }
  }

  // Mega -> Laptop
  while (RS485Serial.available() > 0) {
    String reply = RS485Serial.readStringUntil('\n');
    reply.trim();

    if (reply.length() > 0) {
      Serial.print("Mega: ");
      Serial.println(reply);
    }
  }
}