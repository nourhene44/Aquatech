#include "connection.h"

Connection::Connection()
{

}

bool Connection::createconnect()
{bool test=false;
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName("Taher");//inserer le nom de la source de données
    db.setUserName("hr");//inserer nom de l'utilisateur
    db.setPassword("hr");//inserer mot de passe de cet utilisateur

    if (db.open())
        test=true;





    return  test;
}

QString Connection::lastError() const
{
    return m_lastError;
}

QString Connection::selectedDriver() const
{
    return m_selectedDriver;
}

QStringList Connection::availableDrivers() const
{
    return m_availableDrivers;
}
