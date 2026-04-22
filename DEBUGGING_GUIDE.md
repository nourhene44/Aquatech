# Guide de Dépannage - Système Bateau/Quai

## Problèmes Identifiés et Solutions

### 1. **Arduino n'affiche pas l'ID du quai assez longtemps**
**Cause** : Le code original n'utilisait pas de machine à états, l'afficheur était remis à jour en continu.
**Solution** : ✅ Code Arduino corrigé avec machine à états (fichier arduino_code_corrected.ino)
- Les états sont maintenant : ATTENTE, BATEAU_DETECTE, QUAI_ASSIGNE, PORT_COMPLET
- L'affichage ne change que quand l'état change

### 2. **Qt ne détecte pas les quais libres**
**Cause** : Les quais n'ont probablement pas de statut initialisé ou le statut n'est pas "Libre" exactement.
**Solution** :
- Exécutez le script SQL : `init_quais_statut.sql`
- Vérifiez que tous les quais ont le statut "Libre" initialement

### 3. **Logs de débogage ajoutés**
Pour voir les logs, ouvrez le Output panel dans Qt Creator (Ctrl+Alt+O) ou lancez l'application depuis la ligne de commande.

Les logs afficheront :
```
initializeArduinoLink: Tentative de connexion Arduino
Arduino connecté sur COM3 (exemple)
processArduinoMessage: Message reçu: ARRIVEE Key: arrivee
findFreeQuaiId: Quai 1 Status: Libre Key: libre
findFreeQuaiId: Quai 1 est LIBRE
assignFreeQuaiAndNotifyArduino: Quai 1 trouvé et libre
updateQuaiStatus: Quai 1 mis à jour à Occupe Rows affected: 1
assignFreeQuaiAndNotifyArduino: Envoi message Arduino: "QUAI:1"
```

## Checklist de Vérification

- [ ] Exécuter le script SQL `init_quais_statut.sql` sur la BD Oracle
- [ ] Téléverser le code Arduino corrigé sur l'Arduino
- [ ] Redémarrer l'application Qt
- [ ] Vérifier les logs dans Output panel
- [ ] Tester : Rapprocher un objet du capteur ultrasonic
- [ ] Vérifier que l'Arduino affiche "Quai X"
- [ ] Vérifier que Qt affiche le quai avec statut "Occupe"
- [ ] Éloigner l'objet du capteur
- [ ] Vérifier que l'Arduino affiche "Bateau parti"
- [ ] Vérifier que Qt affiche le quai avec statut "Libre"

## Messages Arduino Attendus

### À la réception de "ARRIVEE":
```
Arduino (vers Qt):
- ARRIVEE

Qt (vers Arduino):
- QUAI:1 (ou le numéro du quai libre)
OU
- COMPLET (si aucun quai libre)
```

### À la réception de "DEPART":
```
Arduino (vers Qt):
- DEPART
```

## Vérification Manuelle en SQL

```sql
-- Voir l'état actuel des quais
SELECT ID_QUAI, NOM_QUAI, STATUT FROM QUAIS ORDER BY ID_QUAI;

-- Réinitialiser tous les quais à "Libre"
UPDATE QUAIS SET STATUT = 'Libre';

-- Mettre manuellement un quai à "Occupé" pour test
UPDATE QUAIS SET STATUT = 'Occupe' WHERE ID_QUAI = 1;
```

## Valeurs de Statut Acceptées

Le système accepte les variantes suivantes (insensible à la casse et accents) :
- **Libre** : "Libre", "libre", "LIBRE", "libéré", etc.
- **Occupé** : "Occupe", "occupe", "OCCUPE", "occupé", etc.

Le code normalise automatiquement les statuts à "Libre" ou "Occupe" en majuscule.
