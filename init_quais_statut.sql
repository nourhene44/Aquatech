-- Script d'initialisation des statuts des quais
-- Exécutez ce script pour réinitialiser tous les quais avec le statut "Libre"

-- Mise à jour de tous les quais à "Libre"
UPDATE QUAIS SET STATUT = 'Libre' WHERE STATUT IS NULL OR STATUT = '';

-- Vérification des données
SELECT ID_QUAI, NOM_QUAI, STATUT FROM QUAIS ORDER BY ID_QUAI;

-- Si vous voulez réinitialiser complètement tous les quais à "Libre":
-- UPDATE QUAIS SET STATUT = 'Libre';
