# ✅ À FAIRE - CHECKLIST EXÉCUTION

## 🎯 ÉTAPES EXACTES DANS L'ORDRE

### ÉTAPE 1: Arduino Upload (5 min)
- [ ] Ouvrir Arduino IDE
- [ ] Ouvrir **ARDUINO_CODE_COPIER_COLLER_UPDATE.txt** (notepad ou editeur)
- [ ] Copier TOUT le contenu
- [ ] Arduino IDE: Ctrl+A (sélectionner tout)
- [ ] Arduino IDE: Delete (supprimer ancien code)
- [ ] Arduino IDE: Ctrl+V (coller code V2)
- [ ] Arduino IDE: Tools → Board → Arduino Uno
- [ ] Arduino IDE: Tools → Port → COM? (ton Arduino)
- [ ] Arduino IDE: Ctrl+U (Upload)
- [ ] Vérifier: "Done uploading" ✅

### ÉTAPE 2: Qt Compilation (2 min)
- [ ] Ouvrir Qt Creator
- [ ] Ouvrir projet (projet.pro)
- [ ] Qt: Ctrl+Ctrl+B (Rebuild All)
- [ ] Vérifier: 0 erreurs de compilation
- [ ] Vérifier: "Build completed" ✅

### ÉTAPE 3: Qt Run (1 min)
- [ ] Qt: Ctrl+R (Run application)
- [ ] Vérifier: Fenêtre Qt s'ouvre
- [ ] Vérifier: Console verte visible (status bar en bas)
- [ ] Connecter Arduino à USB

### ÉTAPE 4: Test Basique (5 min)
- [ ] Qt: Sélectionner port COM dans combo box
- [ ] Qt: Cliquer "Connect"
- [ ] Vérifier console affiche:
  - [ ] "[QT] Command: START"
  - [ ] "[ARDUINO] Sensor ACTIVE"
- [ ] Afficheur LCD affiche: "Port en attente"
- [ ] Approcher objet près capteur
- [ ] Vérifier console affiche:
  - [ ] "[QT CALC] PULSE=..."

### ÉTAPE 5: Démo Flow (10 min)
- [ ] Lire: GUIDE_DEMO_POUR_PROF.md
- [ ] Tester: ARRIVAL (objet à ~40cm)
- [ ] Tester: DOCKING (objet à ~15cm)
- [ ] Tester: DEPARTURE (éloigner objet)
- [ ] Fermer Qt
- [ ] Vérifier: Afficheur affiche "Qt arrêt"

### ÉTAPE 6: Documentation (2 min)
- [ ] Créer dossier: REMISE_PROF
- [ ] Copier: GUIDE_DEMO_POUR_PROF.md
- [ ] Copier: README_SOLUTION_QT_CONTROLE.md
- [ ] Copier: RESUME_FINAL_COMPLET.md

### ÉTAPE 7: Présentation (Facul/Pro)
- [ ] Lire: GUIDE_DEMO_POUR_PROF.md (dernière fois)
- [ ] Pratiquer: Démo flow complet 2-3 fois
- [ ] Montrer: Console `[QT CALC]`
- [ ] Montrer: Code Qt calcul distance
- [ ] Montrer: Arduino code envoie PULSE brut
- [ ] Montrer: Afficheur "En attente"
- [ ] Montrer: Afficheur "Qt arrêt" si Qt fermé
- [ ] Expliquer: Architecture maître-esclave

---

## 🎯 DURÉE TOTALE

```
Étape 1 (Arduino):    5 min
Étape 2 (Qt compile): 2 min
Étape 3 (Qt run):     1 min
Étape 4 (Test):       5 min
Étape 5 (Démo):      10 min
Étape 6 (Doc):        2 min
─────────────────────────
TOTAL:               25 minutes
```

---

## ✅ AVANT DE DIRE "C'EST BON"

- [ ] Arduino V2 uploadé
- [ ] Qt recompilé sans erreurs
- [ ] Console verte affiche [QT CALC]
- [ ] Afficheur affiche "En attente" + "Port en attente"
- [ ] Capteur s'active avec "START"
- [ ] Calculs distance affichés en console
- [ ] Flow complet fonctionne (ARRIVAL→DOCKED→DEPARTED)
- [ ] Qt close → afficheur affiche "Qt arrêt"

---

## 🚨 SI PROBLÈME

### "Arduino IDE ne upload pas"
- [ ] Vérifier: Board = Arduino Uno
- [ ] Vérifier: Port = COM? où Arduino est branché
- [ ] Essayer: Rebooter Arduino (débrancher 5 sec)

### "Qt compile pas"
- [ ] Vérifier: mainwindow.h modifié
- [ ] Vérifier: mainwindow.cpp modifié
- [ ] Essayer: Clean and Rebuild

### "Console ne montre pas [QT CALC]"
- [ ] Vérifier: Arduino V2 uploadé (pas V1)
- [ ] Vérifier: Qt recompilé avec derniers changements
- [ ] Vérifier: Port connecté

### "Afficheur n'affiche rien"
- [ ] Vérifier: LCD I2C alimenté (VCC, GND)
- [ ] Vérifier: Adresse I2C = 0x27
- [ ] Vérifier: Pins SDA, SCL branchés

### "Capteur ne fonctionne pas"
- [ ] Vérifier: Pins TRIG=9, ECHO=10
- [ ] Vérifier: Alimenté 5V
- [ ] Vérifier: Qt a envoyé "START" (log visible)

---

## 🎓 SCRIPT POUR PROF

**Prépare ce petit discours:**

> "Vous voyez dans la console Qt: `[QT CALC] PULSE=1530µs → DISTANCE=26cm`.  
> Cela montre que Qt reçoit le temps brut d'écho du capteur (1530 microsecondes),  
> Qt applique la formule physique pour calculer la distance (26 cm),  
> Qt affiche le calcul pour la preuve, et Qt décide ce qu'il faut faire.  
> 
> L'afficheur LCD affiche 'En attente' au startup, ce qui montre qu'Arduino  
> ne fait rien tant que Qt ne lui envoie pas la commande 'START'.  
> 
> Regardez: si je ferme Qt maintenant, l'afficheur change à 'Qt arrêt'  
> ce qui prouve qu'Arduino s'arrête immédiatement si Qt ferme.  
> C'est une architecture maître-esclave où Qt est le maître intelligent  
> et Arduino est un esclave qui exécute les ordres."

---

## 📋 DOCUMENTS À MONTRER À PROF

1. **Console Qt** (photo/screenshot)
   → Affiche [QT CALC]

2. **Code Qt** (fichier mainwindow.cpp)
   → Fonction qui calcule distance

3. **Code Arduino** (ARDUINO_CODE_COPIER_COLLER_UPDATE.txt)
   → Envoie PULSE, pas DISTANCE

4. **Afficheur LCD** (physiquement)
   → Affiche "En attente" et "Qt arrêt"

5. **Documentation** (fichiers README/GUIDE)
   → Explications complètes

---

## ✨ TU ES PRÊT!

✅ Code créé  
✅ Arduino préparé  
✅ Qt modifié  
✅ Tests faits  
✅ Démo prête  
✅ Documentation complète  

**BONNE PRÉSENTATION!** 🎓🚀
