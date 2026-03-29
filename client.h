#ifndef CLIENT_H
#define CLIENT_H

#include <QString>
#include <QDate>

class client
{
public:
    client();

    // Génère un nouvel ID client selon le schéma 265NNNN (retourne vide si échec)
    static QString genererNouvelId();

    static bool addClient(const QString& id,
                          const QString& nom,
                          const QString& prenom,
                          const QString& profil,
                          const QString& statut,
                          const QDate& date_inscription = QDate(),
                          const QString& telephone = QString());

    static bool updateClient(const QString& id,
                             const QString& nom,
                             const QString& prenom,
                             const QString& profil,
                             const QString& statut,
                             const QDate& date_inscription = QDate(),
                             const QString& telephone = QString());

    static bool deleteClient(const QString& id);

    static QString lastError();
};

#endif // CLIENT_H
