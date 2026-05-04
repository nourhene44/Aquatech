#ifndef CONNECTION_H
#define CONNECTION_H

#include <QDebug>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>

class Connection
{
private:
    static Connection* instance;
    Connection();

    QSqlDatabase db;

public:
    ~Connection();

    static Connection* getInstance();

    bool createconnect(bool showMessages = true);
    void fermerConnexion();
    bool estConnecte();
    QSqlDatabase getDatabase();

    bool ensureOpen();
    QString lastErrorText() const;
    QStringList availableDrivers() const;
    QString selectedDriver() const;
    QString lastError() const;
};

#endif // CONNECTION_H
