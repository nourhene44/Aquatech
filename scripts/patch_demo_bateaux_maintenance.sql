-- AquaTech - Patch des données de maintenance (démo)
-- Objectif: éviter un tableau 100% "Critique" quand les dates sont anciennes (ex: 01/01/2000).
-- Règles côté app (voir computeSeverity):
--   - Critique: maintenance dépassée (daysRemaining < 0)
--   - Urgent:   <= 7 jours
--   - Avert.:   <= 30 jours
--
-- Ce patch positionne 2 bateaux pour obtenir:
--   - 1 bateau "Urgent" (échéance dans 5 jours)
--   - 1 bateau "Avertissement" (échéance dans 20 jours)
--
-- Syntaxe Oracle (DATE + TRUNC(SYSDATE)).

-- Bateau URGENT (dans 5 jours)
UPDATE BATEAUX
SET DATE_DERNIERE_MAINTENANCE = TRUNC(SYSDATE) - (37 - 5),
    PROCHAINE_MAINTENANCE     = 37
WHERE ID_BATEAU = 261009;

-- Bateau AVERTISSEMENT (dans 20 jours)
UPDATE BATEAUX
SET DATE_DERNIERE_MAINTENANCE = TRUNC(SYSDATE) - (50 - 20),
    PROCHAINE_MAINTENANCE     = 50
WHERE ID_BATEAU = 261008;

COMMIT;

-- Vérification
SELECT ID_BATEAU,
       NOM,
       DATE_DERNIERE_MAINTENANCE,
       PROCHAINE_MAINTENANCE AS "Prochaine maintenance",
       (DATE_DERNIERE_MAINTENANCE + PROCHAINE_MAINTENANCE) AS NEXT_DUE,
       TRUNC((DATE_DERNIERE_MAINTENANCE + PROCHAINE_MAINTENANCE) - SYSDATE) AS DAYS_REMAINING
FROM BATEAUX
WHERE ID_BATEAU IN (261008, 261009);
