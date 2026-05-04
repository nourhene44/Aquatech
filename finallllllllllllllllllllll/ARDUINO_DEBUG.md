# Guide de Dépannage Arduino - Page Employé

## ⚠️ Problème Rapporté
"La porte n'ouvre pas et rien ne fonctionne" sur la page employé avec Arduino RFID.

## 🔍 Étapes de Diagnostic

### 1. Vérifier la Connexion Matérielle
```
- Arduino connecté via USB ?
- Câble USB valide ?
- Port COM visible dans Device Manager (Gestionnaire de périphériques) ?
```

### 2. Vérifier le Statut dans l'Application
**Barre de statut en bas à droite** devrait afficher:
- ✅ "RFID: COM4" → Arduino connecté
- ❌ "RFID: disconnected" → Arduino déconnecté
- ❌ "Arduino: No serial ports available" → Aucun port trouvé

### 3. Si "No serial ports available":

**Étape A**: Installer les pilotes USB
- CH340 (Arduino clones): https://sparks.gogo.co.nz/ch340.html
- CP2102 (autres): https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
- FTDI: https://ftdichip.com/drivers/

**Étape B**: Redémarrer l'application
- L'app relance automatiquement la recherche de ports toutes les 5 secondes
- La barre de statut confirmera quand les ports sont trouvés

**Étape C**: Reconnecter l'Arduino
- Débrancher USB, attendre 2 sec, rebrancher
- L'app devrait détecter et se reconnecter automatiquement

### 4. Si "disconnected" s'affiche:

**Étape A**: Cliquer sur "Connect" dans la barre d'outils
- Combo box = Port COM sélectionné
- Bouton "Connect" = Forcer la connexion

**Étape B**: Vérifier le message d'erreur
- Si erreur affichée → voir détails ci-dessous

### 5. Vérifier le Sketch Arduino

Le sketch DOIT:
- Écouter le port série à **9600 bauds**
- Envoyer des UID RFID (hexadécimal)
- Recevoir les commandes: `OPEN:UID` ou `DENY:UID`
- Contrôler la serrure

Si le sketch est manquant → Télécharger et programmer l'Arduino

### 6. Tester Manuellement

**Via la page Employé**:
1. Scanner un badge RFID sur le lecteur
2. La page devrait afficher le UID dans "RFID UID (Scannez...)"
3. Cliquer "Analyser RFID Qt" pour vérifier accès
4. La porte devrait s'ouvrir si accès autorisé

**Via la barre d'outils Arduino** (test technique):
1. Champ "TX line" = Ligne de test
2. Exemple: Taper `OPEN:1A 2B 3C 4D` puis Enter
3. Arduino doit recevoir et traiter la commande

## 🛠️ Messages d'Erreur Courants

| Message | Cause | Solution |
|---------|-------|----------|
| "No serial ports available" | Port COM non détecté | Installer pilotes + rebrancher |
| "Could not select a port" | Aucun port sélectionnable | Redémarrer app + rebrancher |
| "auto-connect failed" | Connexion au port échoue | Vérifier pilotes + port en utilisation |
| "Badge inconnu ou non autorise" | UID pas en base de données | Ajouter badge dans page Admin |
| "Acces refuse: employe banni" | Employé marqué comme banni | Débannir dans page Admin |

## 📊 Flux RFID Complet

```
Arduino (lecteur RFID)
    ↓ (envoie UID par série)
Port COM
    ↓ (reçu par application)
ArduinoSerial::handleIncomingBytes()
    ↓ (extrait le UID)
ArduinoSerial::uidReceived() [signal]
    ↓ (Qt signal/slot)
MainWindow::handleArduinoUid() [slot]
    ↓ (vérifie accès en base)
Employe::canAccessByRfid()
    ↓ (retour allowed=true/false)
ArduinoSerial::sendAccessDecision()
    ↓ (envoie OPEN ou DENY)
Arduino (contrôle serrure)
    ↓ (ouvre ou refuse)
Porte mécanique
```

## ✅ Checklist de Vérification

- [ ] Arduino connecté physiquement via USB
- [ ] Port COM visible dans Device Manager
- [ ] Pilotes USB installés (CH340 / CP2102 / FTDI)
- [ ] Application redémarrée après install pilotes
- [ ] "RFID: COM#" visible dans barre de statut
- [ ] Sketch Arduino chargé et fonctionnel à 9600 baud
- [ ] Badge/UID ajouté dans base de données (Admin)
- [ ] Employé pas marqué comme banni
- [ ] Scanner RFID branchée à l'Arduino
- [ ] Serrure électrique branchée à l'Arduino

## 🔗 Références

- Page Employé: Onglet Employé > Tab RFID
- Config Arduino: Barre d'outils en bas (Port + Connect)
- Logs statut: Barre en bas à droite
- Admin (Ajouter badges): Page Admin > Gestion Employés

