# 🎓 RÉPONSES AUX QUESTIONS PROF

## 🎯 QUESTIONS PROBABLES & RÉPONSES PRÊTES

---

### Q1: "Qui calcule la distance, Arduino ou Qt?"

**Réponse:**
> "Qt calcule la distance. Arduino envoie simplement le temps brut d'écho en microsecondes (PULSE:1530). Qt reçoit ce temps, applique la formule physique `(pulse × 0.034) / 2`, calcule 26cm, et affiche le calcul dans la console: `[QT CALC] PULSE=1530µs → DISTANCE=26cm`. Vous pouvez voir ici dans le code Qt la fonction qui fait ce calcul."

**Où montrer:**
- Console Qt avec `[QT CALC]`
- Code mainwindow.cpp ligne ~5850
- Formule: `int distCm = (pulseMicros * 0.034) / 2.0;`

---

### Q2: "Comment je sais que c'est vraiment Qt et pas Arduino?"

**Réponse:**
> "Trois preuves:
> 1. La console Qt affiche le calcul `[QT CALC]` - c'est du code Qt qui l'écrit
> 2. L'afficheur LCD affiche 'En attente' au startup - c'est Arduino qui attend Qt
> 3. Si je ferme Qt, l'afficheur affiche 'Qt arrêt' - Arduino dépend de Qt
> Arduino ne peut pas fonctionner seul."

**Où montrer:**
- Console verte Qt
- Afficheur LCD au startup
- Fermer Qt et voir afficheur changer

---

### Q3: "Pourquoi Arduino envoie PULSE et pas DISTANCE?"

**Réponse:**
> "Parce que PULSE est la donnée brute du capteur (temps d'écho).  
> Arduino envoie les données brutes, Qt les traite.  
> C'est une séparation claire des responsabilités:  
> Arduino = capteur pur (zéro logique),  
> Qt = intelligence (logique métier)."

**Où montrer:**
- Code Arduino: `Serial.print("PULSE:");`
- Code Qt: formule calcul

---

### Q4: "Qu'est-ce que c'est 'En attente' sur l'écran?"

**Réponse:**
> "C'est l'afficheur LCD qui montre qu'Arduino est en mode 'esclave' attendant  
> une commande de Qt. Quand Qt se connecte, il envoie 'START' et l'afficheur  
> change pour montrer des informations utiles. Ça prouve que l'afficheur  
> est contrôlé par Qt, pas autonome."

**Où montrer:**
- Afficheur au startup
- Logs console: `[QT] Command: START`
- Afficheur change après START

---

### Q5: "Pourquoi envoyer STOP à la fermeture?"

**Réponse:**
> "Pour démontrer que Arduino reçoit et exécute les commandes de Qt.  
> Si Arduino était indépendant, fermer Qt n'aurait aucun effet.  
> Mais regardez: quand je ferme Qt, l'afficheur affiche 'Qt arrêt'.  
> Cela prouve que Arduino exécute la commande STOP de Qt."

**Où montrer:**
- Fermer Qt
- Observer afficheur: "En attente" / "Qt arrêt"
- Console: `[QT] Shutdown: Command STOP sent`

---

### Q6: "Comment fonctionne l'architecture?"

**Réponse:**
> "C'est une architecture maître-esclave:
> - Qt est le MAÎTRE qui:
>   * Envoie commandes (START/STOP)
>   * Reçoit données brutes
>   * Calcule distance
>   * Prend décisions (états)
>   * Gère base de données
> 
> - Arduino est l'ESCLAVE qui:
>   * Attend commandes
>   * Lit capteur
>   * Envoie données brutes
>   * Affiche LCD
>   * Exécute ordres"

**Où montrer:**
- Diagramme architecture (README)
- Flux console complet
- Code Qt qui envoie START/STOP

---

### Q7: "La base de données est où?"

**Réponse:**
> "La base de données Oracle est sur serveur.  
> Qt se connecte en ODBC et exécute des requêtes SQL.  
> Par exemple, quand un bateau est détecté, Qt:
> 1. SELECT FROM QUAIS WHERE STATUT != 'OCCUPEE'
> 2. UPDATE QUAIS SET STATUT = 'OCCUPEE'
> Vous voyez dans la console: `[DB] UPDATE quai SET STATUT=`..."

**Où montrer:**
- Console Qt avec `[DB]` logs
- Code Qt qui fait les requêtes
- Structure DB QUAIS table

---

### Q8: "Quel est le flux complet?"

**Réponse:**
> "Voici le flux:
> 1. Qt démarre → envoie START
> 2. Arduino s'active → envoie PULSE
> 3. Qt reçoit PULSE → calcule distance
> 4. Qt affiche `[QT CALC]`
> 5. Qt compare: si < 50cm → ARRIVAL
> 6. Qt query DB: SELECT quai libre
> 7. Qt envoie BATEAU_DETECTE à Arduino
> 8. Arduino affiche LCD
> 9. Qt continue avec logique...
> 10. Bateau part: Qt envoie BATEAU_PARTI
> 11. Qt libère quai dans DB
> 12. Qt ferme: envoie STOP
> 13. Arduino arrête"

**Où montrer:**
- Console verte affiche tout ce flux
- GUIDE_DEMO_POUR_PROF.md

---

### Q9: "C'est combien la distance?"

**Réponse:**
> "La formule est: distance = (pulse × 0.034) / 2
> Par exemple, si Arduino envoie PULSE:1530 microsecondes,
> Qt calcule: (1530 × 0.034) / 2 = 26 centimètres
> C'est la formule standard pour capteur ultrasonic HC-SR04."

**Où montrer:**
- Formule dans code Qt
- Console exemple: `[QT CALC] PULSE=1530µs → DISTANCE=26cm`

---

### Q10: "Pourquoi pas juste tout en Arduino?"

**Réponse:**
> "Parce que Arduino a des limitations:
> - Pas de vraie base de données
> - Pas de vraie interface utilisateur
> - Calculs limités
> - Pas facile de faire logique complexe
> 
> Qt a:
> - Connection ODBC à base de données
> - Interface riche (console, etc)
> - Logique métier avancée
> - Testabilité
> 
> La séparation est plus claire et plus maintenable."

**Où montrer:**
- Architecture maître-esclave
- Code Qt vs Arduino

---

### Q11: "C'est quoi les états ARRIVAL/DOCKED/DEPARTED?"

**Réponse:**
> "Ce sont les états du bateau basés sur la distance:
> - ARRIVAL: distance < 50cm (bateau arrive)
> - DOCKED: distance < 20cm (bateau amarre)
> - DEPARTED: distance > 80cm (bateau part)
> 
> Qt décide automatiquement l'état et affiche l'action correspondante
> (logs + LCD). Vous voyez dans console:
> `[STATE] ARRIVAL detected`
> `[STATE] DOCKED`
> `[STATE] DEPARTED`"

**Où montrer:**
- Console logs avec [STATE]
- Code mainwindow.cpp qui gère états
- Flow démo

---

### Q12: "Comment vous testé ça?"

**Réponse:**
> "Le test est simple:
> 1. Upload Arduino
> 2. Recompile Qt
> 3. Lancer Qt
> 4. Connecter port COM
> 5. Approcher objet du capteur (~40cm)
> 6. Observer console `[QT CALC]`
> 7. Voir afficheur changer
> 8. Tester tous les états
> 9. Fermer Qt et voir afficheur arrêter
> 
> Tout fonctionne comme prévu."

**Où montrer:**
- Démo en direct
- Logs console
- Afficheur LCD

---

## 📋 PHRASES-CLÉ À UTILISER

```
"Qt est le MAÎTRE"
"Arduino est l'ESCLAVE"
"Consultez la console [QT CALC]"
"L'architecture maître-esclave"
"Qt calcule, Arduino ne fait que..."
"Regardez l'afficheur afficher 'En attente'"
"Quand je ferme Qt, Arduino s'arrête"
"Preuve que tout est Qt"
```

---

## 🎬 POINTS À MONTRER PHYSIQUEMENT

1. **Console Qt verte** → Montrer `[QT CALC]`
2. **Afficheur LCD** → "En attente" + "Qt arrêt"
3. **Code Qt** → Fonction calcul
4. **Code Arduino** → Envoie PULSE
5. **Démo en direct** → Objet proche capteur

---

## ✅ TU ES PRÉPARÉ!

✅ Questions répondues  
✅ Points à montrer listés  
✅ Phrases-clé prêtes  
✅ Démo planifiée  

**BON SUCCÈS!** 🎓✨
