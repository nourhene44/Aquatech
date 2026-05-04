#ifndef BATEAAUUU_H
#define BATEAAUUU_H

#include <QString>
#include <QDate>

class bateaauuu
{
public:
    bateaauuu();

    // Génère un nouvel ID bateau selon le schéma 261NNN (retourne vide si échec)
    static QString genererNouvelId();

    static bool addBateau(const QString& id,
                          const QString& nom,
                          const QString& type,
                          int capacite,
                          const QString& proprietaire,
                          const QString& statut,
                          double largeur,
                          const QDate& date_entree = QDate(),
                          const QDate& date_derniere_maintenance = QDate(),
                          int frequence_maintenance = 0);

    static bool updateBateau(const QString& id,
                             const QString& nom,
                             const QString& type,
                             int capacite,
                             const QString& proprietaire,
                             const QString& statut,
                             double largeur,
                             const QDate& date_entree = QDate(),
                             const QDate& date_derniere_maintenance = QDate(),
                             int frequence_maintenance = 0);

    static bool deleteBateau(const QString& id);

    // Return last error message from the helper (empty on success)
    static QString lastError();
};

#endif // BATEAAUUU_H

