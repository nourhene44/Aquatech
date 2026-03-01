-- ============================================================================
-- AQUATECH - SCRIPT SQL DE CONFIGURATION DES QUOTAS
-- ============================================================================
-- Exécutez ces commandes dans Oracle SQL*Plus ou SQL Developer
-- Utilisateur: HR / Mot de passe: hr
-- Database: Taher (ODBC)
-- ============================================================================

-- 1. CRÉER LA TABLE QUOTAS
-- (Exécutez une seule fois)
CREATE TABLE QUOTAS (
    ID_QUOTA NUMBER PRIMARY KEY,
    TYPE_POISSON VARCHAR2(50) NOT NULL,
    VALEUR_QUOTA NUMBER(10,2) NOT NULL
);

-- 2. AJOUTER LES QUOTAS PAR DÉFAUT
-- (Remarque: les ID et types de poissons doivent correspondre à votre structure)

INSERT INTO QUOTAS (ID_QUOTA, TYPE_POISSON, VALEUR_QUOTA) VALUES (1, 'Sardine', 200);
INSERT INTO QUOTAS (ID_QUOTA, TYPE_POISSON, VALEUR_QUOTA) VALUES (2, 'Maquereau', 150);
INSERT INTO QUOTAS (ID_QUOTA, TYPE_POISSON, VALEUR_QUOTA) VALUES (3, 'Merlu', 180);
INSERT INTO QUOTAS (ID_QUOTA, TYPE_POISSON, VALEUR_QUOTA) VALUES (4, 'Thon', 300);
INSERT INTO QUOTAS (ID_QUOTA, TYPE_POISSON, VALEUR_QUOTA) VALUES (5, 'Loup', 120);
INSERT INTO QUOTAS (ID_QUOTA, TYPE_POISSON, VALEUR_QUOTA) VALUES (6, 'Calamar', 100);
INSERT INTO QUOTAS (ID_QUOTA, TYPE_POISSON, VALEUR_QUOTA) VALUES (7, 'Crevette', 80);
INSERT INTO QUOTAS (ID_QUOTA, TYPE_POISSON, VALEUR_QUOTA) VALUES (8, 'Rouget', 90);
INSERT INTO QUOTAS (ID_QUOTA, TYPE_POISSON, VALEUR_QUOTA) VALUES (9, 'Poulpes', 110);

-- 3. VALIDER LES CHANGEMENTS
COMMIT;

-- ============================================================================
-- VÉRIFICATION - Exécutez ces commandes pour vérifier l'installation
-- ============================================================================

-- Vérifier que la table existe et contient les données
SELECT * FROM QUOTAS;

-- Résultat attendu:
-- ID_QUOTA | TYPE_POISSON | VALEUR_QUOTA
-- ---------|--------------|---------------
-- 1        | Sardine      | 200
-- 2        | Maquereau    | 150
-- 3        | Merlu        | 180
-- 4        | Thon         | 300
-- 5        | Loup         | 120
-- 6        | Calamar      | 100
-- 7        | Crevette     | 80
-- 8        | Rouget       | 90
-- 9        | Poulpes      | 110

-- Compter le nombre de quotas
SELECT COUNT(*) as nombre_quotas FROM QUOTAS;
-- Résultat attendu: 9

-- Vérifier un quota spécifique
SELECT * FROM QUOTAS WHERE TYPE_POISSON = 'Sardine';
-- Résultat attendu: 1, Sardine, 200

-- ============================================================================
-- TESTS DE MODIFICATION (optionnel)
-- ============================================================================

-- Avant modification
SELECT VALEUR_QUOTA FROM QUOTAS WHERE TYPE_POISSON = 'Sardine';
-- Doit retourner: 200

-- Modifier le quota de Sardine à 250
UPDATE QUOTAS 
SET VALEUR_QUOTA = 250 
WHERE TYPE_POISSON = 'Sardine';

-- Valider
COMMIT;

-- Vérifier la modification
SELECT VALEUR_QUOTA FROM QUOTAS WHERE TYPE_POISSON = 'Sardine';
-- Doit retourner: 250

-- Revert à la valeur originale si nécessaire
UPDATE QUOTAS 
SET VALEUR_QUOTA = 200 
WHERE TYPE_POISSON = 'Sardine';
COMMIT;

-- ============================================================================
-- COMMANDES UTILES POUR LE DÉPANNAGE
-- ============================================================================

-- Afficher la structure de la table
DESC QUOTAS;

-- Afficher les colonnes et types
SELECT COLUMN_NAME, DATA_TYPE, NULLABLE 
FROM USER_TAB_COLUMNS 
WHERE TABLE_NAME = 'QUOTAS';

-- Vérifier les contraintes
SELECT * FROM USER_CONSTRAINTS WHERE TABLE_NAME = 'QUOTAS';

-- ============================================================================
-- SI VOUS AVEZ BESOIN DE RÉINITIALISER
-- ============================================================================

-- Pour SUPPRIMER la table (attention: données perdues!)
-- DROP TABLE QUOTAS;

-- Puis recréer selon les instructions ci-dessus

-- ============================================================================
-- NOTES IMPORTANTES
-- ============================================================================
-- 1. Exécutez d'abord le CREATE TABLE (une seule fois)
-- 2. Puis les INSERT pour les données par défaut (une seule fois)
-- 3. Exécutez COMMIT après chaque série de commandes
-- 4. L'application sauvegarde automatiquement les modifications dans cette table
-- 5. À chaque démarrage, l'application charge les données de cette table
-- ============================================================================
