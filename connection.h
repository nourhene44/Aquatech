#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>
#include <QString>

class Connection
{
private:
    // Instance unique (Singleton)
    static Connection* instance;

    // Constructeur privé
    Connection();

    // Objet de connexion à la base de données
    QSqlDatabase db;

public:
    // Destructeur
    ~Connection();

    // Méthode pour obtenir l'instance unique
    static Connection* getInstance();

    // Méthode pour établir la connexion - NOM CORRIGÉ!
    bool createconnect();  // ← Garde ce nom pour correspondre à main.cpp

    // Méthode pour fermer la connexion
    void fermerConnexion();

    // Méthode pour vérifier l'état de la connexion
    bool estConnecte();

    // Méthode pour obtenir la base de données
    QSqlDatabase getDatabase();

    // Helpers used by UI code
    bool ensureOpen();
    QString lastErrorText() const;
    
    // Méthodes pour obtenir les informations du driver
    QStringList availableDrivers() const;
    QString selectedDriver() const;
    QString lastError() const;
};

#endif // CONNECTION_H
