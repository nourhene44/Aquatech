#ifndef CONNECTION_H
#define CONNECTION_H
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QStringList>

class Connection
{
public:
    Connection();
    bool createconnect();

    QString lastError() const;
    QString selectedDriver() const;
    QStringList availableDrivers() const;

private:
    QString m_lastError;
    QString m_selectedDriver;
    QStringList m_availableDrivers;
};

#endif // CONNECTION_H
