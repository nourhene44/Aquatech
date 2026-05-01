/* ==========================================================================
 * AQUATECH - ARDUINO CODE FINAL (SCENARIO BATEAU)
 * ==========================================================================
 * 
 * DESCRIPTION:
 * This is PURE SENSOR & LCD code - NO business logic
 * All decisions are made by Qt application via serial commands
 * 
 * HARDWARE:
 * - Ultrasonic sensor: TRIG pin 9, ECHO pin 10
 * - LCD 16x2 I2C: Address 0x27, SDA/SCL on I2C pins
 * - Serial: 9600 baud
 * 
 * COMMUNICATION PROTOCOL:
 * 
 * Arduino -> Qt:
 *   DISTANCE:<cm>\n     (e.g., "DISTANCE:45")
 *   
 * Qt -> Arduino:
 *   DISPLAY:<text>      (e.g., "DISPLAY:Quai 5")
 *   QUAI:<id>           (e.g., "QUAI:263001")
 *   COMPLET
 *   BATEAU_DETECTE
 *   BATEAU_GARE
 *   BATEAU_PARTI
 * 
 * ========================================================================== */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ========================================================================== 
// PIN CONFIGURATION
// ==========================================================================
#define TRIG_PIN 9
#define ECHO_PIN 10

// ==========================================================================
// LCD (16x2, I2C address 0x27)
// ==========================================================================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ==========================================================================
// TIMING
// ==========================================================================
const unsigned long DISTANCE_SEND_INTERVAL = 500;  // Send sensor reading every 500ms
const unsigned long DISPLAY_UPDATE_INTERVAL = 100; // LCD refresh rate

// ==========================================================================
// STATE
// ==========================================================================
unsigned long lastDistanceSendAt = 0;
unsigned long lastDisplayUpdateAt = 0;
String line1 = "Port en attente";
String line2 = "";

// ==========================================================================
// Read distance in cm
// ==========================================================================
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

// ==========================================================================
// Update LCD display
// ==========================================================================
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

// ==========================================================================
// Process incoming serial message from Qt
// ==========================================================================
void handleMessage(const String& msg) {
  String s = msg;
  s.trim();
  if (s.length() == 0) return;

  String upper = s;
  upper.toUpperCase();

  // Custom display text
  if (upper.startsWith("DISPLAY:")) {
    line1 = s.substring(8);
    line1.trim();
    updateDisplay();
    return;
  }

  // Quai assignment: "QUAI:263001"
  if (upper.startsWith("QUAI:")) {
    line1 = "Id quai:";
    line2 = s.substring(5);
    line2.trim();
    updateDisplay();
    return;
  }

  // All occupied
  if (upper == "COMPLET") {
    line1 = "PORT COMPLET";
    line2 = "Aucun quai libre";
    updateDisplay();
    return;
  }

  // Boat detected (show this when Qt decides)
  if (upper == "BATEAU_DETECTE") {
    line1 = "Bateau detecte!";
    line2 = "Attente quai...";
    updateDisplay();
    return;
  }

  // Boat docked
  if (upper == "BATEAU_GARE") {
    line1 = "Bateau gare";
    line2 = "";
    updateDisplay();
    return;
  }

  // Boat left
  if (upper == "BATEAU_PARTI") {
    line1 = "Bateau parti";
    line2 = "";
    updateDisplay();
    return;
  }
}

// ==========================================================================
// SETUP
// ==========================================================================
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

// ==========================================================================
// MAIN LOOP
// ==========================================================================
void loop() {
  unsigned long now = millis();

  // 1. Send distance reading to Qt every 500ms
  if (now - lastDistanceSendAt >= DISTANCE_SEND_INTERVAL) {
    long dist = readDistanceCm();
    Serial.print("DISTANCE:");
    Serial.println(dist);
    lastDistanceSendAt = now;
  }

  // 2. Process incoming serial messages from Qt
  while (Serial.available()) {
    String incoming = Serial.readStringUntil('\n');
    incoming.trim();
    if (incoming.length() > 0) {
      handleMessage(incoming);
    }
  }

  // 3. Refresh LCD
  updateDisplay();

  delay(10);
}
