/*
 * AQUATECH - Arduino final scenario bateau
 *
 * Version du zip basee sur DISTANCE:<cm>.
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
String line1 = "Port en attente";
String line2 = "";

long readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1;

  return (long)(duration * 0.034 / 2.0);
}

void updateDisplay() {
  if (millis() - lastDisplayUpdateAt < DISPLAY_UPDATE_INTERVAL) return;

  lastDisplayUpdateAt = millis();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);

  if (line2.length() > 0) {
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
}

void handleMessage(const String& msg) {
  String s = msg;
  s.trim();
  if (s.length() == 0) return;

  String upper = s;
  upper.toUpperCase();

  if (upper.startsWith("DISPLAY:")) {
    line1 = s.substring(8);
    line1.trim();
    updateDisplay();
    return;
  }

  if (upper.startsWith("QUAI:")) {
    line1 = "Id quai:";
    line2 = s.substring(5);
    line2.trim();
    updateDisplay();
    return;
  }

  if (upper == "COMPLET") {
    line1 = "PORT COMPLET";
    line2 = "Aucun quai libre";
    updateDisplay();
    return;
  }

  if (upper == "BATEAU_DETECTE") {
    line1 = "Bateau detecte!";
    line2 = "Attente quai...";
    updateDisplay();
    return;
  }

  if (upper == "BATEAU_GARE") {
    line1 = "Bateau gare";
    line2 = "";
    updateDisplay();
    return;
  }

  if (upper == "BATEAU_PARTI") {
    line1 = "Bateau parti";
    line2 = "";
    updateDisplay();
    return;
  }
}

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(50);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  line1 = "Port en attente";
  line2 = "";
  updateDisplay();

  Serial.println("[ARDUINO] Ready");
}

void loop() {
  const unsigned long now = millis();

  if (now - lastDistanceSendAt >= DISTANCE_SEND_INTERVAL) {
    const long dist = readDistanceCm();
    Serial.print("DISTANCE:");
    Serial.println(dist);
    lastDistanceSendAt = now;
  }

  while (Serial.available()) {
    String incoming = Serial.readStringUntil('\n');
    incoming.trim();
    if (incoming.length() > 0) {
      handleMessage(incoming);
    }
  }

  updateDisplay();
  delay(10);
}
