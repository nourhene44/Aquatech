#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define TRIG_PIN 9
#define ECHO_PIN 10

enum DisplayMode {
  ATTENTE,
  BATEAU_DETECTE,
  QUAI_ASSIGNE,
  BATEAU_GARE,
  BATEAU_PARTI,
  PORT_COMPLET
};

DisplayMode currentMode = ATTENTE;
DisplayMode lastMode = ATTENTE;
long assignedQuai = -1;
bool bateauPresent = false;
unsigned long lastDisplayUpdate = 0;
unsigned long lastArriveeSentAt = 0;
unsigned long bateauDetecteAt = 0;
unsigned long departedAt = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 500; // ms
const unsigned long ARRIVEE_RETRY_INTERVAL = 2000; // ms
const unsigned long BATEAU_DETECTE_DISPLAY_MS = 900; // ms
const unsigned long BATEAU_GARE_DISTANCE_CM = 20; // cm
const unsigned long DEPART_DISPLAY_MS = 1800; // ms

bool parseQuaiMessage(const String& input, long* outQuaiId) {
  if (!outQuaiId) return false;
  String s = input;
  s.trim();
  s.replace(" ", "");
  s.toUpperCase();
  if (s.startsWith("QUAI:")) {
    const long id = s.substring(5).toInt();
    if (id > 0) { *outQuaiId = id; return true; }
  }
  if (s.startsWith("IDQUAI:")) {
    const long id = s.substring(7).toInt();
    if (id > 0) { *outQuaiId = id; return true; }
  }

  return false;
}

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(50);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  lcd.init();
  lcd.backlight();
  updateDisplay();
}

long lireDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duree = pulseIn(ECHO_PIN, HIGH);
  return duree * 0.034 / 2;
}

void updateDisplay() {
  if (currentMode == lastMode && (millis() - lastDisplayUpdate < DISPLAY_UPDATE_INTERVAL)) {
    return;
  }
  lastDisplayUpdate = millis();
  lastMode = currentMode;
  lcd.clear();

  switch (currentMode) {
    case ATTENTE:
      lcd.setCursor(0, 0);
      lcd.print("Port en attente");
      break;

    case BATEAU_DETECTE:
      lcd.setCursor(0, 0);
      lcd.print("Bateau detecte!");
      lcd.setCursor(0, 1);
      lcd.print("Attente quai...");
      break;

    case QUAI_ASSIGNE:
      lcd.setCursor(0, 0);
      lcd.print("Id quai:");
      lcd.setCursor(9, 0);
      lcd.print(assignedQuai);
      break;

    case BATEAU_GARE:
      lcd.setCursor(0, 0);
      lcd.print("Bateau gare");
      break;

    case BATEAU_PARTI:
      lcd.setCursor(0, 0);
      lcd.print("Bateau parti");
      lcd.setCursor(0, 1);
      lcd.print("Quai libere");
      break;

    case PORT_COMPLET:
      lcd.setCursor(0, 0);
      lcd.print("PORT COMPLET");
      lcd.setCursor(0, 1);
      lcd.print("Aucun quai libre");
      break;
  }
}

void loop() {
  long distance = lireDistance();

  // === GESTION DE LA DÉTECTION ===
  // Bateau détecté (distance < 50 cm)
  if (distance > 0 && distance < 50 && !bateauPresent) {
    bateauPresent = true;
    
    // Ne changer le mode que si on est en ATTENTE ou PORT_COMPLET
    if (currentMode == ATTENTE || currentMode == PORT_COMPLET) {
      currentMode = BATEAU_DETECTE;
      assignedQuai = -1;
      bateauDetecteAt = millis();
      Serial.println("ARRIVEE");
      lastArriveeSentAt = millis();
    }
  }

  if (bateauPresent && currentMode == BATEAU_DETECTE) {
    const unsigned long now = millis();
    if (now - lastArriveeSentAt >= ARRIVEE_RETRY_INTERVAL) {
      Serial.println("ARRIVEE");
      lastArriveeSentAt = now;
    }
  }

  if (distance > 80 && bateauPresent) {
    bateauPresent = false;
    currentMode = BATEAU_PARTI;
    departedAt = millis();
    assignedQuai = -1;
    Serial.println("DEPART");
  }

  if (currentMode == BATEAU_PARTI && (millis() - departedAt >= DEPART_DISPLAY_MS)) {
    currentMode = ATTENTE;
  }

  // Quand le quai a déjà été reçu, n'afficher l'ID qu'après un court délai
  if (currentMode == BATEAU_DETECTE && assignedQuai > 0) {
    if (millis() - bateauDetecteAt >= BATEAU_DETECTE_DISPLAY_MS) {
      currentMode = QUAI_ASSIGNE;
    }
  }

  if (bateauPresent && assignedQuai > 0 && distance > 0 && distance <= BATEAU_GARE_DISTANCE_CM) {
    currentMode = BATEAU_GARE;
  }

  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();
    String msgKey = msg;
    msgKey.replace(" ", "");
    msgKey.toUpperCase();

    long numQuai = -1;
    if (parseQuaiMessage(msg, &numQuai)) {
      assignedQuai = numQuai;
      if (currentMode != BATEAU_PARTI && currentMode != PORT_COMPLET) {
        if (millis() - bateauDetecteAt >= BATEAU_DETECTE_DISPLAY_MS) {
          currentMode = QUAI_ASSIGNE;
        } else {
          currentMode = BATEAU_DETECTE;
        }
      }
      Serial.println("QUAI_RECEIVED:" + String(numQuai));
    }
    else if (msgKey == "COMPLET") {
      currentMode = PORT_COMPLET;
      Serial.println("PORT_COMPLET_RECEIVED");
    }
    else if (msgKey == "RESET") {
      currentMode = ATTENTE;
      assignedQuai = -1;
      bateauPresent = false;
    }
  }
  updateDisplay();
  delay(100);
}
