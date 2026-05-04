#include "connection.h"

Connection* Connection::instance = nullptr;

Connection::Connection()
{
    db = QSqlDatabase::addDatabase("QODBC");
}

Connection::~Connection()
{
    fermerConnexion();
}

Connection* Connection::getInstance()
{
    if (instance == nullptr) {
        instance = new Connection();
    }
    return instance;
}

bool Connection::createconnect(bool showMessages)
{
    try {
        QString nomUtilisateur = "hr";
        QString motDePasse = "hr"
                             "123";
        QString hote = "localhost";
        int port = 1521;
        QString sid = "XE";

        QString connectionString = QString(
                                       "DRIVER={Oracle in XE};"
                                       "DBQ=%1:%2/%3;"
                                       "UID=%4;"
                                       "PWD=%5;")
                                       .arg(hote)
                                       .arg(port)
                                       .arg(sid)
                                       .arg(nomUtilisateur)
                                       .arg(motDePasse);

        db.setDatabaseName(connectionString);

        if (!db.open()) {
            const QString erreur = db.lastError().text();
            qDebug() << "Connection DB failed:" << erreur;

            if (showMessages) {
                QMessageBox::critical(nullptr,
                                      "Erreur de Connexion",
                                      "Impossible de se connecter a la base de donnees :\n" + erreur);
            }
            return false;
        }

        qDebug() << "Database connected.";
        qDebug() << "Host:" << hote << ":" << port;
        qDebug() << "SID:" << sid;
        qDebug() << "User:" << nomUtilisateur;
        return true;

    } catch (const std::exception& e) {
        qDebug() << "Connection exception:" << e.what();
        if (showMessages) {
            QMessageBox::critical(nullptr,
                                  "Exception",
                                  "Exception lors de la connexion :\n" + QString::fromLocal8Bit(e.what()));
        }
        return false;
    }
}

void Connection::fermerConnexion()
{
    if (db.isOpen()) {
        db.close();
        qDebug() << "Database connection closed.";
    }
}

bool Connection::estConnecte()
{
    return db.isOpen();
}

QSqlDatabase Connection::getDatabase()
{
    return db;
}

bool Connection::ensureOpen()
{
    if (db.isOpen()) return true;

    if (db.open()) return true;

    qDebug() << "ensureOpen(): open failed:" << db.lastError().text();
    return false;
}

QString Connection::lastErrorText() const
{
    return db.lastError().text();
}

QStringList Connection::availableDrivers() const
{
    return QSqlDatabase::drivers();
}

QString Connection::selectedDriver() const
{
    return db.driverName();
}

QString Connection::lastError() const
{
    return db.lastError().text();
}
