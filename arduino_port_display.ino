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
unsigned long departCandidateAt = 0;
unsigned long gareCandidateAt = 0;

const unsigned long DISPLAY_UPDATE_INTERVAL = 150;     // ms
const unsigned long ARRIVEE_RETRY_INTERVAL = 2000;     // ms
const unsigned long BATEAU_DETECTE_DISPLAY_MS = 3000;  // ms (demande: 3 secondes)
const unsigned long DEPART_DISPLAY_MS = 2500;          // ms
const unsigned long DEPART_CONFIRM_MS = 1000;          // ms
const unsigned long GARE_CONFIRM_MS = 600;             // ms
const long BATEAU_DETECTE_DISTANCE_CM = 50;            // cm
const long BATEAU_GARE_DISTANCE_CM = 20;               // cm
const long BATEAU_DEPART_DISTANCE_CM = 80;             // cm

long readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duree = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms
  if (duree == 0) return -1;

  long dist = (long)(duree * 0.034 / 2.0);
  return dist;
}

bool parseQuaiMessage(const String& input, long* outQuaiId) {
  if (!outQuaiId) return false;

  String s = input;
  s.trim();
  if (s.length() == 0) return false;

  String key = s;
  key.toUpperCase();

  String compact = key;
  compact.replace(" ", "");

  if (compact.startsWith("QUAI:")) {
    long id = compact.substring(5).toInt();
    if (id > 0) {
      *outQuaiId = id;
      return true;
    }
  }

  if (compact.startsWith("IDQUAI:")) {
    long id = compact.substring(7).toInt();
    if (id > 0) {
      *outQuaiId = id;
      return true;
    }
  }

  // Compatibility with "id quai:123"
  if (key.startsWith("ID QUAI:")) {
    long id = key.substring(8).toInt();
    if (id > 0) {
      *outQuaiId = id;
      return true;
    }
  }

  return false;
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
      lcd.setCursor(0, 1);
      lcd.print("Quai ");
      lcd.print(assignedQuai > 0 ? String(assignedQuai) : String("?"));
      break;

    case BATEAU_PARTI:
      lcd.setCursor(0, 0);
      lcd.print("Bateau parti");
      break;

    case PORT_COMPLET:
      lcd.setCursor(0, 0);
      lcd.print("PORT COMPLET");
      lcd.setCursor(0, 1);
      lcd.print("Aucun quai libre");
      break;
  }
}

void handleIncomingSerial() {
  if (!Serial.available()) return;

  String msg = Serial.readStringUntil('\n');
  msg.trim();
  if (msg.length() == 0) return;

  String msgKey = msg;
  msgKey.toUpperCase();
  String compact = msgKey;
  compact.replace(" ", "");

  long numQuai = -1;

  if (parseQuaiMessage(msg, &numQuai)) {
    if (!bateauPresent) {
      Serial.println("QUAI_IGNORED_NO_BOAT");
      return;
    }

    assignedQuai = numQuai;

    // Accept quai assignment in all transient modes except full-port mode.
    if (currentMode != PORT_COMPLET) {
      if (millis() - bateauDetecteAt >= BATEAU_DETECTE_DISPLAY_MS) {
        currentMode = QUAI_ASSIGNE;
      } else {
        currentMode = BATEAU_DETECTE;
      }
    }

    Serial.println("QUAI_RECEIVED:" + String(numQuai));
    return;
  }

  if (compact == "COMPLET") {
    assignedQuai = -1;
    currentMode = PORT_COMPLET;
    Serial.println("PORT_COMPLET_RECEIVED");
    return;
  }

  if (compact == "RESET") {
    assignedQuai = -1;
    bateauPresent = false;
    departCandidateAt = 0;
    gareCandidateAt = 0;
    currentMode = ATTENTE;
    lastMode = ATTENTE;
    Serial.println("RESET_OK");
    return;
  }

  Serial.println("UNKNOWN_CMD:" + msg);
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

void loop() {
  long distance = readDistanceCm();
  unsigned long now = millis();
  const bool boatSeenNow = (distance > 0 && distance < BATEAU_DETECTE_DISTANCE_CM);
  const bool boatCloseNow = (distance > 0 && distance <= BATEAU_GARE_DISTANCE_CM);
  const bool boatAbsentNow = (distance < 0) || (distance > BATEAU_DEPART_DISTANCE_CM);

  // 1) Arrival detection
  if (boatSeenNow && !bateauPresent) {
    bateauPresent = true;
    assignedQuai = -1;
    bateauDetecteAt = now;
    departCandidateAt = 0;
    gareCandidateAt = 0;

    if (currentMode == ATTENTE || currentMode == PORT_COMPLET || currentMode == BATEAU_PARTI) {
      currentMode = BATEAU_DETECTE;
    }

    Serial.println("ARRIVEE");
    lastArriveeSentAt = now;
  }

  // 2) ARRIVEE retry while no quai assigned
  if (bateauPresent && assignedQuai <= 0 &&
      (currentMode == BATEAU_DETECTE || currentMode == ATTENTE)) {
    if (now - lastArriveeSentAt >= ARRIVEE_RETRY_INTERVAL) {
      Serial.println("ARRIVEE");
      lastArriveeSentAt = now;
    }
  }

  // 3) Boat docked when very close for a stable interval
  if (bateauPresent && assignedQuai > 0 && boatCloseNow) {
    if (gareCandidateAt == 0) gareCandidateAt = now;
    if (now - gareCandidateAt >= GARE_CONFIRM_MS) {
      currentMode = BATEAU_GARE;
    }
  } else {
    gareCandidateAt = 0;
  }

  // 4) Departure has priority over quai assignment.
  // As soon as the sensor no longer sees the boat, show "Bateau parti" immediately.
  if (bateauPresent && boatAbsentNow) {
    if (currentMode != BATEAU_PARTI) {
      currentMode = BATEAU_PARTI;
      departedAt = now;
      Serial.println("DEPART");
    }

    if (departCandidateAt == 0) departCandidateAt = now;
    if (now - departCandidateAt >= DEPART_CONFIRM_MS) {
      bateauPresent = false;
      assignedQuai = -1;
      gareCandidateAt = 0;
      departCandidateAt = 0;
    }
  } else if (bateauPresent) {
    departCandidateAt = 0;

    // 5) Transition to QUAI_ASSIGNE after the visible detect window.
    if (assignedQuai <= 0) {
      currentMode = BATEAU_DETECTE;
    } else if (boatCloseNow) {
      currentMode = BATEAU_GARE;
    } else if (now - bateauDetecteAt >= BATEAU_DETECTE_DISPLAY_MS) {
      currentMode = QUAI_ASSIGNE;
    } else {
      currentMode = BATEAU_DETECTE;
    }

    if (assignedQuai > 0 && boatCloseNow) {
      currentMode = BATEAU_GARE;
    }
  }

  // 6) End departure display period
  if (currentMode == BATEAU_PARTI && (now - departedAt >= DEPART_DISPLAY_MS)) {
    currentMode = ATTENTE;
  }

  if (!bateauPresent && currentMode != PORT_COMPLET && currentMode != BATEAU_PARTI) {
    currentMode = ATTENTE;
  }

  // 7) Incoming data from Qt
  handleIncomingSerial();

  // 8) LCD refresh
  updateDisplay();
  delay(40);
}
