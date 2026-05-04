/*
 * AQUATECH - Arduino minimal scenario bateau
 *
 * Version simple pour test rapide.
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define TRIG_PIN 9
#define ECHO_PIN 10

LiquidCrystal_I2C lcd(0x27, 16, 2);

const unsigned long DISTANCE_SEND_INTERVAL = 500;
const unsigned long DISPLAY_UPDATE_INTERVAL = 100;

unsigned long lastDistanceSendAt = 0;
unsigned long lastDisplayUpdateAt = 0;
String currentDisplayLine1 = "Port en attente";
String currentDisplayLine2 = "";

long readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duree = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duree == 0) return -1;

  return (long)(duree * 0.034 / 2.0);
}

void updateDisplay() {
  if (millis() - lastDisplayUpdateAt < DISPLAY_UPDATE_INTERVAL) {
    return;
  }
  lastDisplayUpdateAt = millis();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(currentDisplayLine1);

  if (currentDisplayLine2.length() > 0) {
    lcd.setCursor(0, 1);
    lcd.print(currentDisplayLine2);
  }
}

void handleIncomingMessage(const String& message) {
  String msg = message;
  msg.trim();
  if (msg.length() == 0) return;

  String upper = msg;
  upper.toUpperCase();

  if (upper.startsWith("DISPLAY:")) {
    currentDisplayLine1 = msg.substring(8);
    currentDisplayLine2 = "";
    updateDisplay();
    return;
  }

  if (upper.startsWith("QUAI:")) {
    currentDisplayLine1 = "Id quai:";
    currentDisplayLine2 = msg.substring(5);
    updateDisplay();
    return;
  }

  if (upper == "COMPLET") {
    currentDisplayLine1 = "PORT COMPLET";
    currentDisplayLine2 = "Aucun quai libre";
    updateDisplay();
    return;
  }

  if (upper == "BATEAU_DETECTE") {
    currentDisplayLine1 = "Bateau detecte!";
    currentDisplayLine2 = "Attente quai...";
    updateDisplay();
    return;
  }

  if (upper == "BATEAU_GARE") {
    currentDisplayLine1 = "Bateau gare";
    currentDisplayLine2 = "";
    updateDisplay();
    return;
  }

  if (upper == "BATEAU_PARTI") {
    currentDisplayLine1 = "Bateau parti";
    currentDisplayLine2 = "";
    updateDisplay();
    return;
  }
}

void sendDistanceToQt(long distCm) {
  if (distCm < 0) {
    Serial.print("DISTANCE:-1");
  } else {
    Serial.print("DISTANCE:");
    Serial.print(distCm);
  }
  Serial.print("\n");
}

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(50);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  currentDisplayLine1 = "Port en attente";
  currentDisplayLine2 = "";
  updateDisplay();

  Serial.println("[ARDUINO] Ready - waiting for distance readings...");
}

void loop() {
  const unsigned long now = millis();

  if (now - lastDistanceSendAt >= DISTANCE_SEND_INTERVAL) {
    const long distance = readDistanceCm();
    sendDistanceToQt(distance);
    lastDistanceSendAt = now;
  }

  while (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      handleIncomingMessage(line);
    }
  }

  updateDisplay();
  delay(10);
}
