# 🎓 GUIDE DE DÉMONSTRATION POUR TA PROF

## 🎯 OBJECTIF

Démontrer que **Qt contrôle TOUT**, y compris le calcul de la distance.

---

## 🔧 ÉTAPES DE SETUP

### Phase 1: Arduino IDE (5 minutes)
```
1. Ouvrir Arduino IDE
2. Ouvrir le sketch vide
3. Copier-coller ENTIÈREMENT: ARDUINO_CODE_COPIER_COLLER_UPDATE.txt
4. Tools → Board → Arduino Uno
5. Tools → Port → COMx (où x = port du Arduino)
6. Appuyer: Ctrl+U (Upload)
7. Vérifier: "Done uploading" ✅
```

### Phase 2: Qt Application (2 minutes)
```
1. Qt Creator: Ouvrir projet
2. Build → Rebuild All (ou Ctrl+Ctrl+B)
3. Vérifier: Pas d'erreurs de compilation
4. Run (Ctrl+R)
5. Vérifier: Fenêtre Qt s'ouvre avec console verte
```

---

## 🧪 DÉMONSTRATION (5-10 minutes)

### AVANT DE MONTRER À PROF
1. Connecter Arduino via USB
2. Vérifier que serial port apparaît dans combo box Qt
3. Connecter le port (bouton "Connect")

### DÉMO COMPLÈTE

#### Étape 1: Startup (0:00)
**Afficheur LCD:**
```
Port en attente
(quai vide)
```
**Console Qt:**
```
[QT] Connecting to port COM5 9600 baud...
[ARDUINO] Ready signal received
[QT] Command: START → Arduino sensor ACTIVE
[ARDUINO] Sensor ACTIVE
```

**Dit à prof:**
> "Regardez - l'afficheur montre 'Port en attente' au démarrage. Arduino n'est PAS autonome, il attend une commande de Qt. Qt envoie START, l'afficheur confirme en disant Sensor ACTIVE."

---

#### Étape 2: Detection (0:30)
**Action:** Approche ta main à ~40-50cm du capteur

**Afficheur LCD:**
```
Bateau detecte !
Attente quai...
```

**Console Qt:**
```
[QT CALC] PULSE=1530µs → DISTANCE=26cm         ← QT CALCULE ✅
[SENSOR] Distance = 26cm
[STATE] ARRIVAL detected
[DB] SELECT quai libre...
[DB] Selected quai: 263001
[STATE] ARRIVAL => Quai 263001 RESERVE (OCCUPEE)
[TX] BATEAU_DETECTE
[TX] QUAI:263001
```

**Dit à prof:**
> "Regardez la console - Qt REÇOIT PULSE:1530 (temps brut en microsecondes), Qt CALCULE (1530 × 0.034 / 2 = 26cm), Qt AFFICHE le calcul 'QT CALC', Qt DÉCIDE que c'est une ARRIVAL, Qt CHERCHE UN QUAI LIBRE, Qt RÉSERVE LE QUAI. Arduino ne fait que lire le capteur et afficher l'LCD."

---

#### Étape 3: Docking (1:00)
**Action:** Approche ta main à ~15-20cm du capteur

**Afficheur LCD:**
```
Bateau gare !
Quai: 263001
```

**Console Qt:**
```
[QT CALC] PULSE=750µs → DISTANCE=13cm          ← Toujours QT ✅
[SENSOR] Distance = 13cm
[STATE] DOCKED
[TX] BATEAU_GARE
```

**Dit à prof:**
> "Distance réduite, Qt calcule 13cm, Qt décide que le bateau est gare (DOCKED < 20cm), Qt envoie BATEAU_GARE, afficheur affiche le quai assigné. Qt contrôle 100% de la logique."

---

#### Étape 4: Departure (1:30)
**Action:** Éloigne ta main à >80cm du capteur

**Afficheur LCD:**
```
Port en attente
(quai vide)
```

**Console Qt:**
```
[QT CALC] PULSE=2800µs → DISTANCE=48cm         ← Qt continue... ✅
[QT CALC] PULSE=3400µs → DISTANCE=58cm
[QT CALC] PULSE=4200µs → DISTANCE=71cm
[QT CALC] PULSE=5200µs → DISTANCE=88cm         ← >80cm
[SENSOR] Distance = 88cm
[STATE] DEPARTED
[DB] UPDATE quai STATUT='LIBRE'
[STATE] DEPARTED => Quai 263001 LIBERE (LIBRE)
[TX] BATEAU_PARTI
```

**Dit à prof:**
> "Distance augmente, Qt suit en temps réel, Qt détecte que c'est >80cm donc DEPARTED, Qt update la DB pour libérer le quai, Qt envoie BATEAU_PARTI, afficheur revient à Port en attente. TOUTE LA LOGIQUE est en Qt."

---

#### Étape 5: Shutdown (2:00)
**Action:** Fermer application Qt (croix rouge)

**Afficheur LCD:**
```
En attente
Qt arrêt
```

**Console Qt:**
```
[QT] Shutdown: Command STOP sent → Arduino sensor STOPPED
[ARDUINO] Sensor STOPPED
```

**Dit à prof:**
> "En fermant Qt, Qt envoie STOP à Arduino. L'afficheur affiche 'Qt arrêt' ce qui montre que Arduino est maintenant inactif et attend Qt. Le capteur ne fonctionne PLUS. C'est la preuve que Arduino dépend de Qt, pas l'inverse."

---

## 📊 RÉSUMÉ DE LA DÉMO

| Étape | Preuve | Signification |
|-------|--------|--------------|
| 1. Startup | "En attente" + "Sensor ACTIVE" | Qt contrôle activation |
| 2. Detection | `[QT CALC] PULSE=XXX → DISTANCE=26` | Qt CALCULE |
| 3. Docking | `[STATE] DOCKED` + DB update | Qt décide + gère DB |
| 4. Departure | `[STATE] DEPARTED` + DB update | Qt libère DB |
| 5. Shutdown | "Qt arrêt" + "Sensor STOPPED" | Arduino arrête avec Qt |

---

## 💡 POINTS CLÉ POUR PROF

### À Mettre en Avant
1. **Afficheur "En attente"** = Arduino slave (pas indépendant)
2. **Console `[QT CALC]`** = Qt calcule distance (pas Arduino)
3. **Capteur inactive si Qt fermé** = Arduino dépend de Qt
4. **Logs [DB]** = Qt gère base de données
5. **Logs [TX]** = Qt envoie commandes à Arduino

### À Ne PAS Laisser Dire
- ❌ "L'Arduino calcule la distance" (non, Arduino envoie PULSE brut)
- ❌ "L'Arduino fonctionne seul" (non, attend START/STOP de Qt)
- ❌ "C'est du travail Arduino" (non, c'est de l'architecture maître-esclave)

### Réponses Prêtes
**Si prof demande "Pourquoi 'En attente' ?"**
> "Arduino s'attend à une commande de Qt. Si Qt n'envoie pas START, le capteur reste inactif. C'est la preuve que Qt contrôle Arduino."

**Si prof demande "C'est quoi PULSE ?"**
> "Arduino envoie le temps d'écho brut en microsecondes. Qt reçoit ça et calcule la distance avec la formule (pulse × 0.034) / 2. Arduino ne connaît pas la formule, c'est Qt qui l'applique."

**Si prof demande "Pourquoi envoyer STOP ?"**
> "Pour montrer que Arduino écoute Qt. Si Arduino n'écoutait pas, fermer Qt n'aurait aucun effet. Mais l'afficheur change à 'Qt arrêt', ce qui prouve qu'Arduino reçoit et exécute la commande STOP."

---

## 🎬 TIMING SUGGÉRÉ

```
Total: ~10-15 minutes pour démonstration

0:00-1:00  → Setup + Startup
1:00-3:00  → Detection → Docking → Departure
3:00-5:00  → Explications Qt code (show console + code)
5:00-10:00 → Shutdown + discussion
```

---

## 📸 SCREENSHOTS À PRENDRE

Avant la démo, prendre des screenshots de:
1. Console verte Qt avec `[QT CALC]` bien visible
2. Code Qt qui calcule distance
3. Arduino code qui envoie PULSE brut (pas de DISTANCE)

---

## ✅ CHECKLIST AVANT DÉMO

- [ ] Arduino uploadé et COM port visible
- [ ] Qt compilé sans erreurs
- [ ] Capteur HC-SR04 connecté (pins 9, 10)
- [ ] LCD I2C connecté (SDA, SCL)
- [ ] Arduino alimenté
- [ ] Qt applié peut trouver Arduino
- [ ] Console verte visible quand Qt lance
- [ ] Afficheur montre "En attente" au startup

---

## 🚨 TROUBLESHOOTING

### "Afficheur ne montre rien"
- [ ] Vérifier alimentation Arduino
- [ ] Vérifier adresse I2C (0x27)
- [ ] Vérifier pins SDA/SCL

### "Qt ne voit pas Arduino"
- [ ] Vérifier COM port (Tools → Port)
- [ ] Vérifier baud 9600
- [ ] Redémarrer Arduino IDE

### "Console ne montre pas [QT CALC]"
- [ ] Vérifier que code Qt est recompilé
- [ ] Vérifier que Arduino_Code_Copier_Coller_UPDATE.txt est uploadé
- [ ] Redémarrer Qt

---

**Bonne présentation !** 🎓✨
