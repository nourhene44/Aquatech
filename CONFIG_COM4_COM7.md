# Configuration Dual Arduino: COM4 + COM7

## 🎯 Objectif
Fonctionner avec **deux ports Arduino simultanément**:
- **COM4**: RFID (lecteur de badges)
- **COM7**: Capteur de température

## ⚙️ Configuration Appliquée

### 1. Préférences de Port
```
RFID (m_arduino):
  - Port préféré: COM4
  - Fallback: Cherche dans les ports disponibles
  - Reconnexion auto: Toutes les 5 secondes

Température (m_arduinoTemp):
  - Port préféré: COM7 (CHANGÉ de COM5 à COM7)
  - Fallback: Cherche dans les ports disponibles
  - Reconnexion auto: Toutes les 5 secondes
```

### 2. Détection Automatique
```
Démarrage:
  350ms  → Cherche ports RFID (COM4)
  700ms  → Cherche ports Température (COM7)
  
Si aucun port trouvé:
  → Timer 5 sec pour RFID
  → Timer 5 sec pour Température
  → Relance recherche toutes les 5 sec indépendamment
```

### 3. Gestion des Timers
```
Timer RFID:
  - Démarre: Si aucun port trouvé à 350ms
  - S'arrête: Quand connexion réussie
  - Redémarre: Quand connexion perdue

Timer Température:
  - Démarre: Si aucun port trouvé à 700ms
  - S'arrête: Quand connexion réussie
  - Redémarre: Quand connexion perdue
```

### 4. Logique d'Évitement de Conflit
```
Si deux ports disponibles:
  - RFID choisit COM4
  - Température choisit COM7
  → Pas de conflit!

Si seulement COM7 disponible:
  - RFID: Cherche COM4, puis prend COM7
  - Température: Com7 déjà pris? Cherche autres ports
  → Évite les doublons
```

## 🚀 Flux de Démarrage Complet

```
DÉMARRAGE APP
   ↓
0ms: Crée instances m_arduino et m_arduinoTemp
     Crée timers (5 sec chacun)
   ↓
350ms: Cherche ports RFID
   ├─ Ports trouvés (COM4)? → Connecte COM4
   └─ Aucun port? → Démarre timer RFID (5 sec)
   ↓
700ms: Cherche ports Température
   ├─ Ports trouvés (COM7)? → Connecte COM7
   └─ Aucun port? → Démarre timer Température (5 sec)
   ↓
État final:
  ✓ RFID: COM4
  ✓ Température: COM7
  → PRÊT!
```

## 🔌 Scénarios d'Utilisation

### Scénario 1: Les deux Arduino branchés au démarrage
```
Résultat:
  → RFID: COM4 ✓
  → Température: COM7 ✓
  → Immédiatement prêt (< 1 sec)
```

### Scénario 2: Aucun Arduino au démarrage
```
Démarrage:
  → Timers lancés (5 sec chacun)
  
Utilisateur branche les deux Arduino (après 3 sec):
  → Détection après 2 sec (prochaine tentative)
  → RFID: COM4 ✓
  → Température: COM7 ✓
```

### Scénario 3: RFID branché d'abord, Température 5 sec plus tard
```
0-350ms: Rien
  ↓
350ms: RFID (COM4) trouvé ✓
  → Connecte COM4
  → Arrête timer RFID
  ↓
700ms: Température pas trouvée
  → Démarre timer Température (5 sec)
  ↓
+3sec (utilisateur branche Temp):
  → Détection après ≤5 sec
  → Température: COM7 ✓
  → Arrête timer Température
```

### Scénario 4: Un Arduino se débranche
```
EN COURS D'UTILISATION:
  ✓ RFID: COM4
  ✓ Température: COM7
  
Utilisateur débranch COM7:
  → Signal disconnected émis
  → Timer Température relancé (5 sec)
  → Redémarrage barre statut
  
Utilisateur rebranch COM7:
  → Détection après ≤5 sec
  → Reconnexion automatique
  → Redémarrage fonctionnement
```

## 📊 États de Connexion

### Barre de Statut (en bas à droite)
```
RFID: COM4           ← Label combo + bouton Connect/Disconnect
[RFID] Connecté      ← Message statut RFID
[TEMP] Sensor OK     ← Message statut Température
```

### Page Employé
```
⚠ Arduino: déconnecté    (ROUGE)  ← Pas de RFID
✓ Arduino: COM4          (VERT)   ← RFID connecté
```

### Dialog Température
```
Température: 23°C        (VERT)   ← Connecté
Temperature: Port COM7   (ROUGE)  ← Déconnecté
```

## 🔧 Configuration Technique

### Fichiers Modifiés
```
mainwindow.h:
  + QTimer* m_arduinoTempPortDetectionTimer
  + void retryArduinoTemperaturePortDetection()

mainwindow.cpp:
  + preferredTemperaturePortName() → COM5 → COM7
  + ensureArduinoTemperatureConnected() → Ajoute logic timer
  + setupArduinoIntegration() → Crée timer température
  + retryArduinoTemperaturePortDetection() → Nouvelle fonction
```

### Baud Rates (Identiques)
```
RFID: 9600 baud
Température: 9600 baud
```

### Paramètres de Port
```
Data Bits: 8
Stop Bits: 1
Parity: None
Flow Control: None
DTR: Enabled
RTS: Enabled
```

## ✅ Checklist de Vérification

- [ ] COM4 détecté au démarrage
- [ ] COM7 détecté au démarrage
- [ ] RFID: COM4 connecté (label vert)
- [ ] Température: COM7 connecté (dialog)
- [ ] Débrancher COM4 → Redémarrage label rouge
- [ ] Rebrancher COM4 → Reconnexion auto (5 sec max)
- [ ] Débrancher COM7 → Redémarrage dialog rouge
- [ ] Rebrancher COM7 → Reconnexion auto (5 sec max)
- [ ] Scanner RFID fonctionne
- [ ] Lecture température fonctionne
- [ ] Les deux fonctionnent simultanément

## 📞 Dépannage

### RFID ne se connecte pas à COM4
```
Vérifier:
1. Device Manager → Port COM4 visible?
2. Pilotes installés?
3. Autre app utilise COM4?
4. Arduino sketch présent (9600 baud)?
```

### Température ne se connecte pas à COM7
```
Vérifier:
1. Device Manager → Port COM7 visible?
2. Port COM7 pas utilisé par RFID?
3. Capteur branché à COM7?
4. Arduino sketch présent (9600 baud)?
```

### Conflits de Port
```
Solution:
1. Vérifier device manager (COM4 vs COM7)
2. Relancer app
3. Modifier ports préférés si nécessaire
4. Redémarrer les Arduino
```

## 🎯 Résumé

| Aspect | RFID | Température |
|--------|------|-------------|
| **Port Préféré** | COM4 | COM7 |
| **Baud Rate** | 9600 | 9600 |
| **Retry Timer** | 5 sec | 5 sec |
| **Détection Init** | 350ms | 700ms |
| **Reconnexion** | Auto | Auto |
| **Affichage** | Page Employé | Dialog Captures |

---

**Configuration**: COM4 (RFID) + COM7 (Température)
**Mode**: Dual simultané avec retry auto
**Status**: ✅ Prêt pour compilation et test

