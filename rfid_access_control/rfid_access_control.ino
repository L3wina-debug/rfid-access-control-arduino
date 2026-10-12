// RFID access control with an Arduino Uno and an MFRC522 (RC522) reader.
// Green LED + servo "door" for an authorised badge, red LED + buzzer for a refused one.
// Library: MFRC522 by GithubCommunity (Arduino Library Manager). Servo is built in.

#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include "access_logic.h"

const uint8_t PIN_SS = 10, PIN_RST = 9;          // RC522 SDA(SS) and RST
const uint8_t PIN_GREEN = 4, PIN_RED = 5, PIN_BUZZER = 6, PIN_SERVO = 3;
const int SERVO_CLOSED = 0, SERVO_OPEN = 90;
const unsigned long DOOR_OPEN_MS = 3000UL;

// Replace with your badges' UIDs (printed on the Serial Monitor when a card is read).
const CardUid ALLOW_LIST[] = {
  {4, {0xDE, 0xAD, 0xBE, 0xEF}},
  {4, {0x12, 0x34, 0x56, 0x78}},
};
const size_t ALLOW_COUNT = sizeof(ALLOW_LIST) / sizeof(ALLOW_LIST[0]);

MFRC522 reader(PIN_SS, PIN_RST);
Servo door;
Lockout lockout = {0, 0};
CardUid lastUid = {0, {0}};
unsigned long lastReadAt = 0;

void printUid(const CardUid &uid) {
  for (uint8_t i = 0; i < uid.len; i++) {
    if (uid.bytes[i] < 0x10) Serial.print('0');
    Serial.print(uid.bytes[i], HEX);
    if (i + 1 < uid.len) Serial.print(':');
  }
}

void grant() {
  digitalWrite(PIN_GREEN, HIGH);
  door.write(SERVO_OPEN);
  delay(DOOR_OPEN_MS);
  door.write(SERVO_CLOSED);
  digitalWrite(PIN_GREEN, LOW);
}

void refuse(unsigned int beeps) {
  for (unsigned int i = 0; i < beeps; i++) {
    digitalWrite(PIN_RED, HIGH);
    tone(PIN_BUZZER, 1000, 200);
    delay(300);
    digitalWrite(PIN_RED, LOW);
    delay(150);
  }
}

void setup() {
  Serial.begin(9600);
  SPI.begin();
  reader.PCD_Init();
  reader.PCD_SetAntennaGain(reader.RxGain_max);  // better range: reading was sensitive to distance
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  door.attach(PIN_SERVO);
  door.write(SERVO_CLOSED);
  Serial.println(F("RFID access control ready. Present a badge."));
}

void loop() {
  if (!reader.PICC_IsNewCardPresent() || !reader.PICC_ReadCardSerial()) return;

  CardUid uid = {0, {0}};
  uid.len = reader.uid.size > MAX_UID_LEN ? MAX_UID_LEN : reader.uid.size;
  for (uint8_t i = 0; i < uid.len; i++) uid.bytes[i] = reader.uid.uidByte[i];
  reader.PICC_HaltA();
  reader.PCD_StopCrypto1();

  unsigned long now = millis();
  if (isRepeatRead(uid, lastUid, now, lastReadAt)) return;
  lastUid = uid;
  lastReadAt = now;

  Serial.print(F("Card "));
  printUid(uid);

  if (isLocked(lockout, now)) {
    Serial.println(F(" -> LOCKED (too many refused cards)"));
    refuse(1);
    return;
  }

  bool ok = isAuthorized(uid, ALLOW_LIST, ALLOW_COUNT);
  recordResult(lockout, ok, now);
  if (ok) {
    Serial.println(F(" -> ACCESS GRANTED"));
    grant();
  } else {
    Serial.println(F(" -> ACCESS DENIED"));
    refuse(isLocked(lockout, now) ? 3 : 1);
  }
}
