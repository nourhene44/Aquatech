# 🔄 CHANGEMENTS V1 → V2 (FINAL)

## ⚡ RÉSUMÉ RAPIDE

**V1**: Arduino envoie DISTANCE (calculée par Arduino)  
**V2**: Arduino envoie PULSE (temps brut) - **Qt calcule distance**

**V1**: Arduino indépendant, affiche au startup  
**V2**: Arduino esclave, attend "START" de Qt, affiche "En attente"

**V1**: Arduino actif toujours  
**V2**: Arduino inactif quand Qt fermé (capteur arrêté)

---

## 📝 CHANGEMENTS ARDUINO

### V1 Code
```cpp
long readDistanceCm() {
  // Arduino calcule directement la distance
  return (duration * 0.034 / 2.0);  // ← Calcul ici
}

Serial.print("DISTANCE:");
Serial.println(dist);  // ← Envoie valeur calculée
```

### V2 Code
```cpp
long readRawPulseMicros() {
  if (!sensorActive) return -1;  // ← Attend START
  // Arduino envoie temps BRUT
  return (long)duration;  // ← Pas de calcul
}

Serial.print("PULSE:");
Serial.println(pulse);  // ← Envoie temps microseconds
```

### Startup

**V1:**
```
Arduino démarre → affiche "Port en attente" → capteur actif
```

**V2:**
```
Arduino démarre → affiche "En attente / Qt démarre..." 
→ Attend "START" de Qt → capteur inactif
```

### Shutdown

**V1:**
```
Qt ferme → Arduino continue (capteur actif)
```

**V2:**
```
Qt ferme → envoie "STOP" → Arduino s'arrête → affiche "En attente / Qt arrêt"
```

---

## 🖥️ CHANGEMENTS QT

### mainwindow.h
```cpp
// Ajout:
int m_lastRawPulseMicros = -1;  // Stocke temps brut Arduino
```

### mainwindow.cpp - destructeur
```cpp
// Nouveau:
if (m_arduino && m_arduino->isConnected()) {
    m_arduino->writeLine("STOP");  // Arrête Arduino
}
```

### mainwindow.cpp - connected signal
```cpp
// Nouveau:
if (m_arduino) m_arduino->writeLine("START");  // Démarre Arduino
m_portConsole->appendPlainText("[QT] Command: START → Arduino sensor ACTIVE");
```

### mainwindow.cpp - handleArduinoPortLine()

**V1:**
```cpp
if (msgOrig.startsWith("DISTANCE:")) {
    distCm = msgOrig.mid(9).toInt();  // Lit distance déjà calculée
}
```

**V2:**
```cpp
if (msgOrig.startsWith("PULSE:")) {
    int pulse = msgOrig.mid(6).toInt();
    
    // === QT CALCULE ===
    int distCm = (pulse * 0.034) / 2.0;  // ← CALCUL ICI EN QT
    
    m_portConsole->appendPlainText(
        "[QT CALC] PULSE=%1µs → DISTANCE=%2cm"
        .arg(pulse).arg(distCm)  // ← Affiche calcul
    );
}
```

---

## 📊 CONSOLE QT - COMPARAISON

### V1 Logs
```
[SENSOR] 45cm
[STATE] ARRIVAL
```

### V2 Logs
```
[QT] Command: START → Arduino sensor ACTIVE    ← Qt commande Arduino
[ARDUINO] Sensor ACTIVE                        ← Arduino répond
[QT CALC] PULSE=1530µs → DISTANCE=26cm         ← Qt CALCULE
[SENSOR] Distance = 26cm
[STATE] ARRIVAL
```

**V2 monttre clairement que Qt calcule!**

---

## 📋 FICHIERS AFFECTÉS

### Arduino
```
AVANT: ARDUINO_CODE_COPIER_COLLER.txt (V1)
APRÈS: ARDUINO_CODE_COPIER_COLLER_UPDATE.txt (V2)
       ou ARDUINO_CODE_FINAL_QT_CONTROLE.txt (V2)
```

### Qt
```
mainwindow.h     ✅ +1 variable
mainwindow.cpp   ✅ +3 modifications (destructor, connected, handleArduinoPortLine)
```

---

## 🎯 POINTS CLÉS POUR PROF

### Arduino V2
- ✅ Envoie PULSE (temps brut microsecondes)
- ✅ Attend "START" de Qt pour s'activer
- ✅ S'arrête quand Qt envoie "STOP"
- ✅ Affiche "En attente" tant que Qt absent
- ✅ **ZÉRO logique** (capteur pur)

### Qt
- ✅ **Envoie "START" au démarrage**
- ✅ **Calcule distance** à partir du temps brut
- ✅ **Affiche le calcul** dans console : `[QT CALC] PULSE=XXXµs → DISTANCE=YYcm`
- ✅ **Envoie "STOP" à fermeture**
- ✅ Toute la logique (états, DB, etc.)

---

## 🔬 FORMULE DE CALCUL (en Qt)

```
distance(cm) = (pulse(microsecondes) × 0.034) / 2

Exemple:
PULSE:1530 → (1530 × 0.034) / 2 = 26.01cm ≈ 26cm

Logs Qt:
[QT CALC] PULSE=1530µs → DISTANCE=26cm
```

---

## 🚀 MIGRATION V1 → V2

1. **Arduino IDE:**
   - Remplacer code avec `ARDUINO_CODE_COPIER_COLLER_UPDATE.txt`
   - Upload

2. **Qt:**
   - Les modifications sont **DÉJÀ APPLIQUÉES**
   - Recompiler (Ctrl+B)

3. **Test:**
   - Lancer Qt
   - Connecter Arduino
   - Observer console : `[QT CALC] PULSE=XXX...`
   - Montrer à prof que Qt calcule ✅

---

## ✨ AVANTAGES V2

| Aspect | V1 | V2 |
|--------|----|----|
| Calcul distance | Arduino ❌ | Qt ✅ |
| Afficheur | Actif startup | En attente ✅ |
| Capteur actif | Toujours | Uniquement si Qt ✅ |
| Preuve Qt contrôle | Pas visible | Console montre [QT CALC] ✅ |
| Si Qt fermé | Arduino continue | Arduino arrête ✅ |

---

**V2 prouve 100% que tout est Qt !** 🎯
