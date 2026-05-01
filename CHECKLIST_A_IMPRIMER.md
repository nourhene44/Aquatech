# 📋 CHECKLIST À IMPRIMER

## ✅ AVANT DÉMARRAGE

```
□ Arduino branché à USB
□ Qt Creator ouvert
□ Arduino IDE ouvert
□ Afficheur LCD branché
```

## ✅ ÉTAPE 1: ARDUINO (5 min)

```
□ Ouvrir ARDUINO_CODE_COPIER_COLLER_UPDATE.txt
□ Copier le code entièrement
□ Arduino IDE: Ctrl+A (sélectionner tout)
□ Arduino IDE: Delete (supprimer ancien)
□ Arduino IDE: Ctrl+V (coller nouveau)
□ Arduino IDE: Board = Arduino Uno
□ Arduino IDE: Port = COM?
□ Arduino IDE: Upload (Ctrl+U)
□ Vérifier: "Done uploading" ✅
```

## ✅ ÉTAPE 2: QT COMPILE (2 min)

```
□ Qt Creator ouvert
□ Projet ouvert (projet.pro)
□ Rebuild All (Ctrl+Ctrl+B)
□ Vérifier: 0 erreurs compilation
```

## ✅ ÉTAPE 3: QT RUN (1 min)

```
□ Qt: Run (Ctrl+R)
□ Vérifier: Console verte visible
□ Vérifier: Fenêtre Qt s'ouvre
```

## ✅ ÉTAPE 4: CONNEXION (2 min)

```
□ Arduino connecté USB
□ Qt: Sélectionner port COM
□ Qt: Cliquer "Connect"
□ Console affiche: "[QT] Command: START"
□ Afficheur LCD affiche: "En attente"
```

## ✅ ÉTAPE 5: TEST DISTANCE (3 min)

```
□ Approcher objet près capteur (~40cm)
□ Console affiche: "[QT CALC] PULSE=XXX → DISTANCE=YY"
□ Afficheur LCD affiche: "Bateau detecte !"
□ Éloigner objet
□ Console affiche: "[STATE] DEPARTED"
```

## ✅ ÉTAPE 6: ARRÊT (1 min)

```
□ Fermer Qt (croix rouge)
□ Afficheur LCD affiche: "Qt arrêt"
□ Console affiche: "[QT] Shutdown: STOP sent"
```

## ✅ AVANT REMISE À PROF

```
□ Arduino V2 uploadé
□ Qt recompilé
□ Console [QT CALC] visible
□ Afficheur "En attente" visible
□ Afficheur "Qt arrêt" visible (après fermeture)
□ Flow complet testé
```

## ✅ POINTS À MONTRER À PROF

```
□ Console Qt avec [QT CALC] PULSE=...
□ Code Qt qui calcule distance
□ Code Arduino qui envoie PULSE
□ Afficheur LCD "En attente"
□ Démo complète (ARRIVAL→DOCKED→DEPARTED)
```

## ✅ DOCUMENTS À AVOIR

```
□ GUIDE_DEMO_POUR_PROF.md (lire avant présentation)
□ README_SOLUTION_QT_CONTROLE.md (pour prof)
□ RESUME_FINAL_COMPLET.md (backup explications)
```

---

## ⏱️ TIMING

```
Arduino Upload:      5 min
Qt Compile:          2 min
Qt Run:              1 min
Test:                5 min
Présentation:       10 min
─────────────────────────
TOTAL:              23 minutes
```

---

## 🆘 SI ERREUR

```
Arduino ne upload pas?
  □ Vérifier Board = Arduino Uno
  □ Vérifier Port = COM?
  □ Redémarrer Arduino

Qt compile pas?
  □ Vérifier mainwindow.h modifié
  □ Vérifier mainwindow.cpp modifié
  □ Clean and Rebuild

Console ne montre pas [QT CALC]?
  □ Vérifier Arduino V2 uploadé (pas V1)
  □ Vérifier Qt recompilé
  □ Vérifier port connecté

Afficheur ne marche pas?
  □ Vérifier alimentation LCD (VCC, GND)
  □ Vérifier adresse I2C = 0x27
  □ Vérifier pins SDA, SCL
```

---

## 🎓 DISCOURS PROF (30 secondes)

```
"Qt reçoit le temps brut du capteur (PULSE:1530),
calcule la distance (1530 × 0.034 / 2 = 26cm),
affiche le calcul en console [QT CALC],
et prend les décisions.

L'afficheur 'En attente' montre qu'Arduino
dépend de Qt. Si je ferme Qt, Arduino s'arrête
et afficheur affiche 'Qt arrêt'.

C'est une architecture maître-esclave où
Qt est maître et Arduino est esclave."
```

---

**IMPRIMER ET GARDER SUR TOI !** 🎓
