/*
 * RFID scenario controlled by Qt.
 *
 * Qt decides:
 * - OPEN:C3FC0512  -> good badge
 * - DENY:<uid>     -> bad badge
 *
 * Arduino behavior:
 * - Good badge: door opens, green LED on, one short beep.
 * - Bad badge: door stays closed, red LED on briefly, one longer beep.
 */

#include <SPI.h>
#include <MFRC522.h>

constexpr byte SS_PIN = 10;
constexpr byte RST_PIN = 9;
constexpr byte RELAY_PIN = 7;
constexpr byte GREEN_LED_PIN = 6;
constexpr byte RED_LED_PIN = 5;
constexpr byte BUZZER_PIN = 4;

constexpr bool RELAY_ACTIVE_LOW = true;
constexpr unsigned long DOOR_OPEN_MS = 3000;
constexpr unsigned long RED_LED_MS = 1000;
constexpr unsigned long CARD_REPEAT_COOLDOWN_MS = 1500;
constexpr unsigned long UID_COMMAND_TIMEOUT_MS = 5000;

MFRC522 mfrc522(SS_PIN, RST_PIN);

String serialBuffer;
String pendingUidCompact;
unsigned long pendingUidAt = 0;
String lastScanUidCompact;
unsigned long lastScanAt = 0;

bool doorUnlocked = false;
unsigned long doorUnlockAt = 0;
unsigned long redLedUntil = 0;

bool isHexChar(char c) {
  return (c >= '0' && c <= '9') ||
         (c >= 'A' && c <= 'F') ||
         (c >= 'a' && c <= 'f');
}

String normalizeUidCompact(String text) {
  String out;
  out.reserve(text.length());

  for (unsigned int i = 0; i < text.length(); ++i) {
    const char c = text.charAt(i);
    if (isHexChar(c)) {
      out += (char)toupper(c);
    }
  }

  return out;
}

String uidCompactToSpaced(const String& compactUid) {
  String out;
  for (unsigned int i = 0; i < compactUid.length(); ++i) {
    if (i > 0 && (i % 2) == 0) {
      out += ' ';
    }
    out += compactUid.charAt(i);
  }
  return out;
}

void setRelayState(bool unlocked) {
  const uint8_t activeState = RELAY_ACTIVE_LOW ? LOW : HIGH;
  const uint8_t idleState = RELAY_ACTIVE_LOW ? HIGH : LOW;
  digitalWrite(RELAY_PIN, unlocked ? activeState : idleState);
}

void lockDoor() {
  doorUnlocked = false;
  setRelayState(false);
  digitalWrite(GREEN_LED_PIN, LOW);
}

void unlockDoor() {
  doorUnlocked = true;
  doorUnlockAt = millis();
  redLedUntil = 0;
  digitalWrite(RED_LED_PIN, LOW);
  setRelayState(true);
  digitalWrite(GREEN_LED_PIN, HIGH);
}

void beepOnce(unsigned int durationMs) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(durationMs);
  digitalWrite(BUZZER_PIN, LOW);
}

void signalAuthorizedBadge(const String& uidCompact) {
  unlockDoor();
  beepOnce(120);
  Serial.print("ACCESS_GRANTED:");
  Serial.println(uidCompactToSpaced(uidCompact));
}

void signalDeniedBadge(const String& uidCompact) {
  lockDoor();
  digitalWrite(RED_LED_PIN, HIGH);
  redLedUntil = millis() + RED_LED_MS;
  beepOnce(320);
  Serial.print("ACCESS_DENIED:");
  Serial.println(uidCompactToSpaced(uidCompact));
}

void clearPendingUid() {
  pendingUidCompact = "";
  pendingUidAt = 0;
}

void emitUidToQt(const String& uidCompact) {
  pendingUidCompact = uidCompact;
  pendingUidAt = millis();
  lastScanUidCompact = uidCompact;
  lastScanAt = pendingUidAt;

  Serial.print("UID:");
  Serial.println(uidCompactToSpaced(uidCompact));
}

String readCardUidCompact() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return "";
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return "";
  }

  String uidCompact;
  for (byte i = 0; i < mfrc522.uid.size; ++i) {
    if (mfrc522.uid.uidByte[i] < 0x10) {
      uidCompact += '0';
    }
    uidCompact += String(mfrc522.uid.uidByte[i], HEX);
  }

  uidCompact.toUpperCase();
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  return uidCompact;
}

void handleScannedBadge(const String& uidCompact) {
  if (uidCompact.length() == 0) {
    return;
  }

  const unsigned long nowMs = millis();
  if (uidCompact == lastScanUidCompact &&
      lastScanAt > 0 &&
      (nowMs - lastScanAt) < CARD_REPEAT_COOLDOWN_MS) {
    return;
  }

  emitUidToQt(uidCompact);
}

void handleDecisionLine(const String& rawLine) {
  String line = rawLine;
  line.trim();
  if (line.length() == 0) {
    return;
  }

  String upper = line;
  upper.toUpperCase();

  bool allow = false;
  if (upper.startsWith("OPEN")) {
    allow = true;
  } else if (!upper.startsWith("DENY")) {
    return;
  }

  const int colonIndex = upper.indexOf(':');
  const String providedUid = colonIndex >= 0
      ? normalizeUidCompact(upper.substring(colonIndex + 1))
      : "";

  const unsigned long nowMs = millis();
  const bool pendingStillFresh =
      pendingUidCompact.length() > 0 &&
      pendingUidAt > 0 &&
      (nowMs - pendingUidAt) <= UID_COMMAND_TIMEOUT_MS;

  String targetUid = providedUid;
  if (targetUid.length() == 0 && pendingStillFresh) {
    targetUid = pendingUidCompact;
  }

  if (targetUid.length() == 0) {
    return;
  }

  if (pendingStillFresh &&
      pendingUidCompact.length() > 0 &&
      providedUid.length() > 0 &&
      providedUid != pendingUidCompact) {
    return;
  }

  clearPendingUid();

  if (allow) {
    signalAuthorizedBadge(targetUid);
    return;
  }

  signalDeniedBadge(targetUid);
}

void readSerialCommands() {
  while (Serial.available() > 0) {
    const char ch = (char)Serial.read();
    if (ch == '\r') {
      continue;
    }

    if (ch == '\n') {
      handleDecisionLine(serialBuffer);
      serialBuffer = "";
      continue;
    }

    serialBuffer += ch;
    if (serialBuffer.length() > 80) {
      serialBuffer = "";
    }
  }
}

void updateOutputs() {
  const unsigned long nowMs = millis();

  if (doorUnlocked && (nowMs - doorUnlockAt) >= DOOR_OPEN_MS) {
    lockDoor();
  }

  if (redLedUntil > 0 && nowMs >= redLedUntil) {
    digitalWrite(RED_LED_PIN, LOW);
    redLedUntil = 0;
  }
}

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  lockDoor();
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  Serial.println("RFID_READY");
}

void loop() {
  readSerialCommands();
  handleScannedBadge(readCardUidCompact());
  updateOutputs();
}
