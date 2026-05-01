/*
 * ============================================================================
 * AQUATECH - ARDUINO CODE (MINIMAL - SCENARIO BATEAU)
 * ============================================================================
 * 
 * ROLE: Pure sensor reading and LCD display
 * - Reads distance from ultrasonic sensor
 * - Sends raw distance to Qt every 500ms
 * - Receives LCD display commands from Qt
 * - NO database logic - all handled by Qt
 * - NO complex state management - Qt controls everything
 * 
 * COMMUNICATION PROTOCOL:
 * Arduino -> Qt:  
 *   DISTANCE:<cm>     (e.g., "DISTANCE:45")
 *   
 * Qt -> Arduino:
 *   DISPLAY:<text>    (e.g., "DISPLAY:Port en attente")
 *   QUAI:<id>         (e.g., "QUAI:263001")
 *   COMPLET           (No free quai)
 *   BATEAU_DETECTE    (Show: "Bateau detecte!")
 *   BATEAU_GARE       (Show: "Bateau gare")
 *   BATEAU_PARTI      (Show: "Bateau parti")
 * 
 * ============================================================================
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ============================================================================
// PIN CONFIGURATION
// ============================================================================
#define TRIG_PIN 9
#define ECHO_PIN 10

// ============================================================================
// LCD CONFIGURATION (16x2 display at address 0x27)
// ============================================================================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ============================================================================
// TIMING CONSTANTS
// ============================================================================
const unsigned long DISTANCE_SEND_INTERVAL = 500;  // ms - send raw distance every 500ms
const unsigned long DISPLAY_UPDATE_INTERVAL = 100; // ms - update LCD refresh rate

// ============================================================================
// GLOBAL STATE
// ============================================================================
unsigned long lastDistanceSendAt = 0;
unsigned long lastDisplayUpdateAt = 0;
String currentDisplayLine1 = "Port en attente";
String currentDisplayLine2 = "";

// ============================================================================
// FUNCTION: Read distance from ultrasonic sensor
// Returns: distance in cm, or -1 if timeout/error
// ============================================================================
long readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duree = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duree == 0) return -1;

  long dist = (long)(duree * 0.034 / 2.0);
  return dist;
}

// ============================================================================
// FUNCTION: Update LCD display with current text
// ============================================================================
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

// ============================================================================
// FUNCTION: Parse incoming serial messages from Qt
// Expected formats:
//   DISPLAY:<text>
//   QUAI:<id>
//   COMPLET
//   BATEAU_DETECTE
//   BATEAU_GARE
//   BATEAU_PARTI
// ============================================================================
void handleIncomingMessage(const String& message) {
  String msg = message;
  msg.trim();
  if (msg.length() == 0) return;

  String upper = msg;
  upper.toUpperCase();

  // Custom display message from Qt
  if (upper.startsWith("DISPLAY:")) {
    currentDisplayLine1 = msg.substring(8);
    updateDisplay();
    return;
  }

  // Quai assignment: "QUAI:263001"
  if (upper.startsWith("QUAI:")) {
    String quaiId = msg.substring(5);
    currentDisplayLine1 = "Id quai:";
    currentDisplayLine2 = quaiId;
    updateDisplay();
    return;
  }

  // All quais occupied
  if (upper == "COMPLET") {
    currentDisplayLine1 = "PORT COMPLET";
    currentDisplayLine2 = "Aucun quai libre";
    updateDisplay();
    return;
  }

  // Boat detected
  if (upper == "BATEAU_DETECTE") {
    currentDisplayLine1 = "Bateau detecte!";
    currentDisplayLine2 = "Attente quai...";
    updateDisplay();
    return;
  }

  // Boat parked at quai
  if (upper == "BATEAU_GARE") {
    currentDisplayLine1 = "Bateau gare";
    currentDisplayLine2 = "";
    updateDisplay();
    return;
  }

  // Boat left the port
  if (upper == "BATEAU_PARTI") {
    currentDisplayLine1 = "Bateau parti";
    currentDisplayLine2 = "";
    updateDisplay();
    return;
  }
}

// ============================================================================
// FUNCTION: Send raw distance to Qt
// Format: "DISTANCE:<cm>\n"
// ============================================================================
void sendDistanceToQt(long distCm) {
  if (distCm < 0) {
    // Sensor error - send -1
    Serial.print("DISTANCE:-1");
  } else {
    Serial.print("DISTANCE:");
    Serial.print(distCm);
  }
  Serial.print("\n");
}

// ============================================================================
// SETUP
// ============================================================================
void setup() {
  Serial.begin(9600);
  Serial.setTimeout(50);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Show startup message
  currentDisplayLine1 = "Port en attente";
  currentDisplayLine2 = "";
  updateDisplay();

  Serial.println("[ARDUINO] Ready - waiting for distance readings...");
}

// ============================================================================
// MAIN LOOP
// ============================================================================
void loop() {
  unsigned long now = millis();

  // ========================================================================
  // 1. Read distance and send to Qt periodically
  // ========================================================================
  if (now - lastDistanceSendAt >= DISTANCE_SEND_INTERVAL) {
    long distance = readDistanceCm();
    sendDistanceToQt(distance);
    lastDistanceSendAt = now;
  }

  // ========================================================================
  // 2. Process incoming serial messages from Qt
  // ========================================================================
  while (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      handleIncomingMessage(line);
    }
  }

  // ========================================================================
  // 3. Update LCD display
  // ========================================================================
  updateDisplay();

  // Small delay to prevent CPU saturation
  delay(10);
}
