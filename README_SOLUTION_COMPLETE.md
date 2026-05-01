# AQUATECH - SOLUTION COMPLÈTE SCENARIO BATEAU

## SOMMAIRE
- [Vue d'ensemble](#vue-densemble)
- [Code Arduino](#code-arduino)
- [Modifications Qt](#modifications-qt)
- [Protocole de Communication](#protocole-de-communication)
- [Flux Complet](#flux-complet)
- [Installation](#installation)

---

## Vue d'ensemble

Cette solution implémente le scénario de détection automatique de bateau avec :
- **Arduino** : Capteur ultrason + LCD + Communication série (ZÉRO logique métier)
- **Qt** : Toute la logique (détection états, DB queries, assignation quais)
- **Base de données Oracle** : Gestion des quais (statut, ID bateau)

### Principe fondamental
L'Arduino envoie **UNIQUEMENT** les distances brutes du capteur.
Qt décide tout (détection arrivée, amarrage, départ) et commande l'affichage LCD.

---

## Code Arduino

### À utiliser dans Arduino IDE

**Copier-coller le contenu complet du fichier `Arduino_Final_Code.ino`**

**Dépendances requises :**
- `LiquidCrystal_I2C` (gérer via Arduino IDE Library Manager)

**Configuration matériel :**
- Capteur ultrason : `TRIG=PIN9`, `ECHO=PIN10`
- LCD I2C : Adresse `0x27`, connexion `SDA/SCL`
- Baud rate : `9600`

**Fonctionnement :**
1. Lit distance du capteur toutes les 500ms
2. Envoie format `DISTANCE:<cm>\n` (ex: `DISTANCE:45`)
3. Attend commandes Qt et affiche sur LCD
4. Aucune logique de détection/filtrage

---

## Modifications Qt

### Fichiers modifiés

#### 1. **mainwindow.h**
✅ Ajout variables membre pour suivi état bateau:
```cpp
int m_lastKnownDistanceCm = -1;
int m_currentAssignedQuaiId = -1;
bool m_boatPresent = false;
bool m_boatDocked = false;
unsigned long m_arrivalDetectedAt = 0;
QPlainTextEdit* m_portConsole = nullptr;
```

✅ Ajout signatures méthodes:
```cpp
void handleArduinoPortLine(const QString& line);
int findAndReserveFreeQuai();
void markQuaiFree(int idQuai);
```

#### 2. **mainwindow.cpp**

✅ **setupArduinoIntegration()** - Ajout console logs
```cpp
// Console de logs pour messages Arduino/DB (dans la status bar)
m_portConsole = new QPlainTextEdit(this);
m_portConsole->setReadOnly(true);
m_portConsole->setMaximumHeight(120);
// [...]
sb->addWidget(m_portConsole, 10);
```

✅ **findAndReserveFreeQuai()**
- Requête SQL universelle (compatible Oracle, SQLite, PostgreSQL)
- Cherche premier quai libre
- Update statut à 'OCCUPEE'
- Logs dans console

✅ **markQuaiFree(int idQuai)**
- Update statut à 'LIBRE'
- Reset ID_BATEAU à NULL
- Logs dans console

✅ **handleArduinoPortLine()** - ENTIÈREMENT REMPLACÉE
- Parse `DISTANCE:<cm>` depuis Arduino
- Machine à états basée sur distance :
  - **< 50cm** → ARRIVEE (cherche quai, envoie `QUAI:<id>` ou `COMPLET`)
  - **< 20cm** → DOCKING (envoie `BATEAU_GARE`)
  - **> 80cm** → DEPARTURE (envoie `BATEAU_PARTI`, libère quai)
- Logs détaillés : `[SENSOR]`, `[STATE]`, `[DB]`

#### 3. **connection.cpp** / **quai.cpp**
- Requêtes SQL adaptées pour universalité DB
- Utilise `COALESCE` au lieu de `NVL` (plus portable)
- Utilise `LIMIT 1` au lieu de `ROWNUM = 1`

---

## Protocole de Communication

### Arduino → Qt
```
DISTANCE:<cm>\n

Exemples:
DISTANCE:45
DISTANCE:15
DISTANCE:-1  (capteur erreur)
```

### Qt → Arduino
```
DISPLAY:<text>           Afficher texte personnalisé
QUAI:<id>                Quai assigné (ex: QUAI:263001)
COMPLET                  Aucun quai libre
BATEAU_DETECTE           Bateau détecté (commande Qt)
BATEAU_GARE              Bateau amarré (commande Qt)
BATEAU_PARTI             Bateau parti (commande Qt)
```

### Thresholds de Distance
- **Détection** : < 50cm
- **Amarrage** : < 20cm
- **Départ** : > 80cm

---

## Flux Complet

### 1. État initial
```
Arduino: LCD affiche "Port en attente"
Qt: Console affiche "[INIT] Console logs Arduino/Database"
```

### 2. Bateau approche
```
Capteur lit 45cm
Arduino envoie: DISTANCE:45
Qt console: [SENSOR] 45cm
Qt décide: ARRIVEE (< 50cm)
Qt console: [STATE] ARRIVAL
Qt query DB: SELECT quai libre
DB retourne: quai 263001
Qt update DB: STATUT='OCCUPEE' pour 263001
Qt console: [STATE] ARRIVAL => Quai 263001 RESERVE (OCCUPEE)
Qt envoie Arduino: BATEAU_DETECTE
Arduino LCD: "Bateau detecte! / Attente quai..."
Qt envoie Arduino: QUAI:263001
Arduino LCD: "Id quai: / 263001"
```

### 3. Bateau s'approche davantage
```
Capteur lit 18cm
Arduino envoie: DISTANCE:18
Qt console: [SENSOR] 18cm
Qt décide: DOCKING (< 20cm && boat_present && !boat_docked)
Qt console: [STATE] DOCKED
Qt envoie Arduino: BATEAU_GARE
Arduino LCD: "Bateau gare"
```

### 4. Bateau s'éloigne
```
Capteur lit 95cm
Arduino envoie: DISTANCE:95
Qt console: [SENSOR] 95cm
Qt décide: DEPARTURE (> 80cm && boat_present)
Qt console: [STATE] DEPARTED
Qt envoie Arduino: BATEAU_PARTI
Arduino LCD: "Bateau parti"
Qt query DB: UPDATE quai 263001 STATUT='LIBRE'
Qt console: [ARDUINO] DEPART => Quai 263001 LIBERE (LIBRE)
```

### 5. Tous les quais occupés
```
Si aucun quai libre trouvé au step 2:
Qt envoie Arduino: COMPLET
Arduino LCD: "PORT COMPLET / Aucun quai libre"
```

---

## Installation

### 1. Arduino IDE
- Copier `Arduino_Final_Code.ino` dans l'IDE
- Installer bibliothèque `LiquidCrystal_I2C`
- Configurer port COM et carte
- Upload vers Arduino

### 2. Qt Application
- Les modifications sont **DÉJÀ appliquées** aux fichiers :
  - `mainwindow.h`
  - `mainwindow.cpp`
- Recompiler le projet Qt
- Vérifier que la console de logs apparaît dans la status bar

### 3. Base de Données
- S'assurer que table `QUAIS` existe avec colonnes :
  ```
  ID_QUAI (clé primaire)
  STATUT (VARCHAR: 'LIBRE', 'OCCUPEE')
  ID_BATEAU (nullable - foreign key ou NULL)
  [autres colonnes existantes...]
  ```

### 4. Connexion série
- Lancer l'app Qt
- Sélectionner port COM dans status bar (combo box)
- Cliquer "Connect"
- Observer les logs dans la console verte

---

## Dépannage

### Arduino n'envoie rien
- Vérifier pins TRIG (9) et ECHO (10)
- Vérifier baud rate 9600
- Utiliser moniteur série Arduino IDE pour tester

### Qt reçoit distances mais pas de changement LCD
- Vérifier adresse I2C LCD : `0x27` (tester avec scanner I2C)
- Vérifier pins SDA/SCL connectées

### DB query erreur
- Vérifier connexion Oracle dans `Connection::createconnect()`
- Observer logs dans console Qt : `[DB] Erreur...`
- Vérifier table `QUAIS` existe et a colonnes requises

### Logs ne s'affichent pas en Qt
- Console doit être visible dans status bar (fond noir, texte vert)
- Si invisible, redémarrer app
- Vérifier setupArduinoIntegration() appelé au startup

---

## Notes Importantes

✅ **Arduino est pur capteur** - Aucune logique métier
✅ **Qt maîtrise tout** - Détection, DB, assignation  
✅ **DB transparent** - Qt gère les requêtes
✅ **Logs détaillés** - Console facilite débogage
✅ **Scalable** - Ajouter capteurs = ajouter DISTANCE:<n> Arduino
✅ **Robuste** - Gestion erreurs sensors et DB

---

**Développé pour AquaTech - Gestion Port de Pêche**
