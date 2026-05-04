# Fixes Arduino RFID Page Employé

## 🔧 Modifications Effectuées

### 1. **Indicateur d'État RFID sur la Page Employé**
- Ajout d'un label `label_rfid_status` visible sur la page employé
- **Affichage**:
  - ✓ Arduino: COM4 (vert) = Connecté
  - ⚠ Arduino: déconnecté (rouge) = Déconnecté
- **Position**: Sous le champ "RFID UID (Scannez...)"

### 2. **Détection Automatique Périodique des Ports**
- Timer toutes les 5 secondes qui relance la recherche de ports
- **Démarrage**: Quand aucun port trouvé au démarrage
- **Arrêt**: Quand la connexion Arduino réussit
- **Redémarrage**: Quand la connexion est perdue

### 3. **Meilleurs Messages d'Erreur**
```
Application startup:
  → "Arduino: No serial ports available" (8 sec)
  → "Arduino: Could not select a port" (6 sec)
  → Retry toutes les 5 secondes

During retry:
  → "Arduino auto-reconnect failed on COM#: ERROR" (6 sec)

Status bar messages:
  → "[RFID] Arduino connecté sur COM4 (9600 bauds)"
  → "Acces autorise pour John Doe (mode Qt)"
  → "Badge inconnu ou non autorise"
```

### 4. **Mise à Jour Dynamique du Statut**
- Le label se met à jour **immédiatement** quand Arduino se connecte/déconnecte
- Appelé depuis les signaux `connected()` et `disconnected()`

### 5. **Initialisation au Démarrage**
- Le statut RFID est mis à jour quand la page employé est affichée
- Appelé depuis `loadEmployes()` et `on_p6b_clicked()`

## 📍 Fichiers Modifiés

### mainwindow.h
- Ajout: `QTimer* m_arduinoPortDetectionTimer`
- Ajout: `void retryArduinoPortDetection()`
- Ajout: `void updateEmployeePageRfidStatus()`

### mainwindow.cpp
- `setupArduinoIntegration()` (ligne ~5299-5301)
  - Création du timer de retry (5 sec)
  
- Auto-reconnection logic (ligne ~5451-5474)
  - Start timer si aucun port
  - Stop timer si ports trouvés
  
- Signal handlers
  - `connected()` → appelle `updateEmployeePageRfidStatus()`
  - `disconnected()` → appelle `updateEmployeePageRfidStatus()`
  
- `ensureEmployeRfidUi()` (ligne ~3485-3495)
  - Crée le label `label_rfid_status` sur la page
  
- `updateEmployeePageRfidStatus()` (nouvell fonction, ligne ~6483-6500)
  - Met à jour le label avec le statut Arduino
  
- `on_p6b_clicked()` (ligne ~3062)
  - Appelle `updateEmployeePageRfidStatus()` quand page ouverte

## 🧪 Test de la Correction

### Étape 1: Démarrage de l'Application
```
✓ Barre de statut affiche: "Arduino: No serial ports available"
✓ Page employé → label rouge "⚠ Arduino: déconnecté"
✓ Timer lance automatiquement retry (message tous les 5 sec)
```

### Étape 2: Connecter l'Arduino
```
✓ Brancher Arduino via USB
✓ Après 5 secondes max:
  - Barre de statut: "Arduino connecté sur COM4 (9600 bauds)"
  - Page employé label devient vert: "✓ Arduino: COM4"
  - Timer de retry s'arrête
```

### Étape 3: Scanner une Badge RFID
```
✓ Aller à la page Employé
✓ Utiliser le scanner RFID sur un badge valide
✓ Le UID doit s'afficher dans "RFID UID (Scannez...)"
✓ La page devrait traiter automatiquement l'accès
✓ Message: "Acces autorise pour Prenom Nom (mode Qt)"
```

### Étape 4: Tester la Déconnexion
```
✓ Débrancher l'Arduino
✓ Label devient rouge: "⚠ Arduino: déconnecté"
✓ Timer redémarre (retry toutes les 5 sec)
✓ Rebrancher Arduino → Reconnecte automatiquement
```

## ⚙️ Flux Complet du Dépannage

```
Démarrage app
   ↓
350ms: Cherche ports → aucun trouvé
   ↓
Lance timer (5 sec)
   ↓
Utilisateur branche Arduino
   ↓
Timer détecte ports
   ↓
Tente connexion
   ↓
✓ Connexion réussie
   ↓
Label "✓ Arduino: COM4" (vert)
   ↓
Prêt pour scanner RFID
```

## 🔍 Diagnostic si Ça Ne Marche Pas

### Si le label ne change pas de couleur
1. Vérifier que le label s'affiche bien (rouge) au démarrage
2. Brancher/débrancher l'Arduino → label devrait réagir
3. Si rien ne change → Arduino ne se connecte pas

### Si Arduino ne se connecte toujours pas
1. **Port COM introuvable?**
   - Device Manager → Ports
   - Chercher "USB Serial" ou "Arduino"
   - Installer les pilotes si nécessaire

2. **Port détecté mais connexion échoue?**
   - Vérifier le sketch Arduino (9600 baud)
   - Tester via la barre d'outils Arduino (TX field)

3. **Message "auto-connect failed"?**
   - Voir le message d'erreur exact
   - Peut indiquer: Port occupé, Sketch absent, Baud rate incorrect

### Si le scanner RFID ne fonctionne pas
1. Vérifier que le label montre "✓ Arduino: COM#" (connexion OK)
2. Tester la barre d'outils RFID (TX field)
3. Vérifier que le badge est en base de données (Admin)
4. Vérifier que l'employé n'est pas marqué comme "Banni"

## 📞 Support

Si le problème persiste:
1. Noter le message exact de la barre de statut
2. Vérifier les pilotes USB (Device Manager)
3. Redémarrer l'application
4. Rebrancher l'Arduino
5. Vérifier le sketch Arduino sur la carte

