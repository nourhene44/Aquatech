# Guide de Gestion des Quotas - Aquatech

## Description

La fonctionnalité de gestion des quotas permet de :
- **Stocker** les valeurs de quotas de poissons dans la base de données Oracle
- **Charger** automatiquement les quotas lors de l'ouverture de l'application
- **Modifier** les valeurs de quotas et les rendre **persistantes** dans la base de données
- **Consulter** les quotas actualisés lors de chaque réouverture de l'application

## Installation et Configuration

### 1. Création de la table QUOTAS

Avant d'utiliser cette fonctionnalité, vous devez créer la table QUOTAS dans votre base de données Oracle :

```sql
CREATE TABLE QUOTAS (
    ID_QUOTA NUMBER PRIMARY KEY,
    TYPE_POISSON VARCHAR2(50) NOT NULL,
    VALEUR_QUOTA NUMBER(10,2) NOT NULL
);
```

Vous pouvez exécuter le script fourni : `init_quotas.sql`

### 2. Insérer les quotas par défaut

Exécutez les commandes SQL suivantes pour insérer les quotas initials :

```sql
INSERT INTO QUOTAS (ID_QUOTA, TYPE_POISSON, VALEUR_QUOTA) VALUES (1, 'Sardine', 200);
INSERT INTO QUOTAS (ID_QUOTA, TYPE_POISSON, VALEUR_QUOTA) VALUES (2, 'Maquereau', 150);
INSERT INTO QUOTAS (ID_QUOTA, TYPE_POISSON, VALEUR_QUOTA) VALUES (3, 'Merlu', 180);
INSERT INTO QUOTAS (ID_QUOTA, TYPE_POISSON, VALEUR_QUOTA) VALUES (4, 'Thon', 300);
INSERT INTO QUOTAS (ID_QUOTA, TYPE_POISSON, VALEUR_QUOTA) VALUES (5, 'Loup', 120);
INSERT INTO QUOTAS (ID_QUOTA, TYPE_POISSON, VALEUR_QUOTA) VALUES (6, 'Calamar', 100);
INSERT INTO QUOTAS (ID_QUOTA, TYPE_POISSON, VALEUR_QUOTA) VALUES (7, 'Crevette', 80);
INSERT INTO QUOTAS (ID_QUOTA, TYPE_POISSON, VALEUR_QUOTA) VALUES (8, 'Rouget', 90);
INSERT INTO QUOTAS (ID_QUOTA, TYPE_POISSON, VALEUR_QUOTA) VALUES (9, 'Poulpes', 110);
COMMIT;
```

## Utilisation

### Accès à la gestion des quotas

1. Depuis le menu principal, cliquez sur le bouton de "Gestion des Captures" (p5b)
2. Un tableau intitulé "Quotas" s'affichera à gauche, contenant :
   - **Colonne 1** : Type de Poisson (Sardine, Maquereau, Merlu, etc.)
   - **Colonne 2** : Valeur du Quota (en kg)

### Modifier un quota

1. **Sélectionnez** une ligne dans le tableau des quotas (ex: Sardine)
2. Cliquez sur le bouton **✏️ Modifier Quota**
3. Une boîte de dialogue vous demande de confirmer
4. Au succès, un message confirme la mise à jour

**Alternative** (édition directe) :
- Les valeurs des quotas sont **éditables** directement dans les cellules du tableau
- Double-cliquez sur une cellule de quota pour éditer la valeur
- Après modification manuelle, cliquez sur **✏️ Modifier Quota** pour sauvegarder

## Structure des Fichiers

### Nouveaux fichiers ajoutés :
- **quotas.h** : En-tête de la classe Quotas (opérations CRUD)
- **quotas.cpp** : Implémentation des opérations CRUD
- **init_quotas.sql** : Script SQL pour initialiser la table

### Fichiers modifiés :
- **GQuai.h** : Ajout de méthodes pour charger et modifier les quotas
- **GQuai.cpp** : Implémentation de `loadQuotasTable()` et `modifyQuotaRow()`
- **mainwindow.ui** : Ajout du bouton "✏️ Modifier Quota"
- **projet.pro** : Ajout de quotas.cpp et quotas.h à la compilation

## Structure de la classe Quotas

```cpp
class Quotas {
    // Data
    int idQuota;
    QString typePoisson;
    double valeurQuota;
    
    // Opérations CRUD
    bool ajouter();       // Créer
    bool modifier();      // Mettre à jour
    bool supprimer();     // Supprimer
    
    // Lectures statiques
    static QList<QStringList> getQuotasByType();
    static QList<QStringList> getAllQuotasAsRows();
};
```

## Comportement

### Au démarrage de l'application
- Les quotas actuels dans la base de données sont chargés automatiquement
- Le tableau affiche toutes les valeurs sauvegardées

### À la modification
- Les changements sont **immédiatement sauvegardés** dans la base de données
- Lors de la réouverture de l'application, les nouvelles valeurs sont affichées

### Gestion des erreurs
- Si la connexion à la BD échoue, un message d'erreur s'affiche
- Si une valeur invalide est saisie, l'application refuse la modification

## Exemple de flux

1. Utilisateur ouvre l'application
2. Navigue vers "Gestion des Captures"
3. Voit les quotas actuels (ex: Sardine = 200 kg)
4. Sélectionne la ligne Sardine
5. Clique sur "✏️ Modifier Quota"
6. Confirme la modification
7. Ferme l'application
8. Rouvre l'application
9. **Les quotas restent à jour** ✅

## Notes Techniques

- La classe `Quotas` utilise Qt SQL pour les opérations sur la base de données
- Les requêtes sont préparées avec `QSqlQuery::prepare()` pour éviter les injections SQL
- Les committed sont faits après chaque modification pour la persistance
- Error handling complète avec messages utilisateur

## Dépannage

### Les quotas ne se chargent pas
- Vérifiez que la table QUOTAS existe dans la base de données
- Assurez-vous que la connexion ODBC "Taher" est configurée
- Vérifiez les permissions de l'utilisateur DB

### L'erreur "Impossible de modifier le quota" apparaît
- Vérifiez la syntaxe du quota (doit être un nombre)
- Assurez-vous que la base de données n'est pas verrouillée
- Vérifiez les logs d'application pour plus de détails

### Le bouton ne répond pas
- Assurez-vous d'avoir sélectionné une ligne dans le tableau
- Vérifiez que le tableau n'est pas vide
