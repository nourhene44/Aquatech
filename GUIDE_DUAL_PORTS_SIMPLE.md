# 🔌 Guide Rapide: COM4 + COM7 Simultanément

## ✅ Configuration Effectuée

Vous avez maintenant **deux ports Arduino qui fonctionnent en même temps**:

| Port | Fonction | Appareil |
|------|----------|----------|
| **COM4** | RFID (lecteur badges) | Arduino lecteur RFID |
| **COM7** | Capteur de température | Arduino capteur temp |

## 🚀 Comment Ça Marche

### Au Démarrage de l'App
```
1. App démarre
2. Cherche COM4 (RFID) → 350ms
3. Cherche COM7 (Température) → 700ms
4. Si trouvés → Connecte automatiquement
5. Si pas trouvés → Cherche toutes les 5 secondes
```

### Pendant l'Utilisation
```
Les deux ports fonctionnent indépendamment:
  - COM4: Scanner des badges RFID (page employé)
  - COM7: Lecture température (page captures)
  
Ils ne se gênent PAS - font tout en même temps!
```

## 📍 Où Voir le Statut

### Page Employé (RFID)
```
Label en haut à droite:
  ✓ Arduino: COM4     (VERT = connecté)
  ⚠ Arduino: déconnecté (ROUGE = pas connecté)
```

### Page Captures (Température)
```
Dialog "Température":
  Température: 23°C    (VERT = connecté)
  Température: COM7 (erreur: ...)  (ROUGE = erreur)
```

### Barre de Statut (tout en bas)
```
Messages de l'app:
  [RFID] Arduino connecté sur COM4 (9600 bauds)
  [TEMP] Arduino temperature connected: COM7
```

## 🔄 Scénarios

### ✅ Les deux Arduino branchés au démarrage
```
Résultat:
  → Connecte COM4 (< 1 sec)
  → Connecte COM7 (< 1 sec)
  → PRÊT!
```

### ✅ Branchement après démarrage
```
App démarre:
  → Label rouge "⚠ Arduino: déconnecté"
  
Vous branchez les deux Arduino:
  → Détection automatique après ≤5 sec
  → Label vert "✓ Arduino: COM4"
  → Temperature: "COM7 ok"
  → PRÊT! (Sans redémarrer l'app)
```

### ✅ Un Arduino se débranche
```
RFID se débranche:
  → Label devient rouge
  → Cherche COM4 automatiquement
  
Vous rebranchez RFID:
  → Label redevient vert après ≤5 sec
  → Reconnexion auto (pas d'intervention)
```

## 🛠️ Configuration Technique

**Ce qui a changé**:
1. Port Température: COM5 → **COM7**
2. Ajout retry timer pour RFID ✓ (fait)
3. Ajout retry timer pour COM7 ✓ (fait)
4. Gestion indépendante des deux ports ✓ (fait)

**Ports Préférés**:
```
RFID:       COM4 (priorité 1) → autres ports (priorité 2)
Température: COM7 (priorité 1) → autres ports (priorité 2)
```

**Retry Automatique**:
```
Intervalle: 5 secondes
Démarre: Si port pas trouvé
S'arrête: Si connexion réussie
Redémarre: Si connexion perdue
```

## ⚙️ Avant de Compiler

### Vérifier que Vous Avez
- [ ] COM4 sur le gestionnaire de périphériques
- [ ] COM7 sur le gestionnaire de périphériques
- [ ] Pilotes USB installés (CH340, CP2102, FTDI)
- [ ] Arduino RFID programmé (9600 baud)
- [ ] Arduino Température programmé (9600 baud)

### Après Compilation

Test 1: **Connexion au démarrage**
```
1. Brancher COM4 + COM7
2. Lancer l'app
3. Vérifier:
   ✓ Label "✓ Arduino: COM4" (page employé)
   ✓ Dialog "COM7" (page captures)
```

Test 2: **Branchement après démarrage**
```
1. Lancer l'app (aucun Arduino)
   → Label rouge, Dialog rouge
2. Brancher COM4
   → Label vert après 5 sec
3. Brancher COM7
   → Dialog vert après 5 sec (depuis moment branché)
```

Test 3: **Déconnexion/Reconnexion**
```
1. Débrancher COM4
   → Label rouge après 1-2 sec
2. Rebrancher COM4
   → Label vert après 5 sec max
3. Débrancher COM7
   → Dialog rouge après 1-2 sec
4. Rebrancher COM7
   → Dialog vert après 5 sec max
```

Test 4: **Fonctionnalités**
```
1. Scanner un badge (page employé)
   → Vérifier que COM4 fonctionne
2. Lire température (page captures)
   → Vérifier que COM7 fonctionne
3. Faire les deux en même temps
   → Aucune interference
```

## 🔧 Si Ça Ne Marche Pas

### COM4 Non Détecté
```
1. Vérifier Device Manager → "Ports"
2. Voir si COM4 existe
3. Si absent → Driver manquant
   - Installer CH340 ou CP2102
   - Redémarrer app
```

### COM7 Non Détecté
```
1. Vérifier Device Manager → "Ports"
2. Voir si COM7 existe (NOT COM4!)
3. Si absent → Vérifier deuxième Arduino
4. Si COM5 au lieu de COM7:
   - Modifier cable USB
   - Redémarrer app
```

### Les deux ports sur le même COM
```
⚠️ Problème: Deux Arduino sur même port
Solution:
1. Débrancher les deux
2. Rebrancher UN seul (doit être COM4)
3. Attendre ≤5 sec → Connecte
4. Rebrancher le DEUXIÈME (doit être COM7)
5. Attendre ≤5 sec → Connecte
```

### Retry Timer ne s'arrête pas (Cherche éternellement)
```
⚠️ Rare, mais si ça arrive:
1. Redémarrer l'app
2. Brancher les deux Arduino
3. Attendre 10 sec
4. Si toujours en rouge:
   - Vérifier les sketches Arduino
   - Vérifier les baudrates (doit être 9600)
```

## 📞 Récapitulatif

```
AVANT (ancien):
  COM4: RFID
  COM5: Température

APRÈS (nouveau):
  COM4: RFID
  COM7: Température ← CHANGÉ!

Avantages:
  ✓ Deux ports toujours opérationnels
  ✓ Retry automatique si débranché
  ✓ Pas d'intervention manuelle
  ✓ Reconnexion en ≤5 secondes
```

---

**Prêt à compiler et tester!** 🎯

