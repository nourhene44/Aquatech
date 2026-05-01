#include "connection.h"

// Initialisation de l'instance statique à nullptr
Connection* Connection::instance = nullptr;

// Constructeur privé
Connection::Connection() {
    // Ajout du driver Oracle (ODBC)
    db = QSqlDatabase::addDatabase("QODBC");
}

// Destructeur
Connection::~Connection() {
    fermerConnexion();
    if (instance) {
        delete instance;
        instance = nullptr;
    }
}

// Méthode pour obtenir l'instance unique
Connection* Connection::getInstance() {
    if (instance == nullptr) {
        instance = new Connection();
    }
    return instance;
}

// Méthode pour établir la connexion - AVEC LE BON NOM!
bool Connection::createconnect() {
    try {
        // 🔧 PARAMÈTRES DE CONNEXION - À MODIFIER SELON VOTRE CONFIG
        QString nomUtilisateur = "esra";           // Nom d'utilisateur Oracle
        QString motDePasse = "esra123";                  // Mot de passe Oracle
        QString hote = "localhost";                  // Adresse du serveur
        int port = 1521;                             // Port Oracle
        QString sid = "XE";                          // SID de votre base (XE, ORCL, etc.)

        // Méthode 1: Chaîne de connexion ODBC complète (recommandée)
        QString connectionString = QString(
                                       "DRIVER={Oracle in XE};"            // Driver Oracle détecté sur votre système
                                       "DBQ=%1:%2/%3;"                     // Format: hote:port/sid
                                       "UID=%4;"
                                       "PWD=%5;"
                                       ).arg(hote).arg(port).arg(sid).arg(nomUtilisateur).arg(motDePasse);

        db.setDatabaseName(connectionString);

        // Tentative d'ouverture de la connexion
        if (!db.open()) {
            QString erreur = db.lastError().text();
            qDebug() << "❌ Erreur de connexion à la base de données : " << erreur;

            QMessageBox::critical(nullptr,
                                  "Erreur de Connexion",
                                  "Impossible de se connecter à la base de données :\n" + erreur);
            return false;
        }

        qDebug() << "✅ Connexion à la base de données établie avec succès !";
        qDebug() << "📌 Hôte : " << hote << ":" << port;
        qDebug() << "📌 SID : " << sid;
        qDebug() << "👤 Utilisateur : " << nomUtilisateur;

        QMessageBox::information(nullptr,
                                 "Connexion Réussie",
                                 "✅ Connexion à la base de données établie avec succès !");

        return true;

    } catch (const std::exception &e) {
        qDebug() << "❌ Exception lors de la connexion : " << e.what();
        QMessageBox::critical(nullptr,
                              "Exception",
                              "Exception lors de la connexion :\n" + QString(e.what()));
        return false;
    }
}

// Méthode pour fermer la connexion
void Connection::fermerConnexion() {
    if (db.isOpen()) {
        db.close();
        qDebug() << "🔌 Connexion à la base de données fermée.";
    }
}

// Méthode pour vérifier l'état de la connexion
bool Connection::estConnecte() {
    return db.isOpen();
}

// Méthode pour obtenir la base de données
QSqlDatabase Connection::getDatabase() {
    return db;
}

bool Connection::ensureOpen() {
    if (db.isOpen()) return true;

    // If createconnect() was called, connection parameters are already set.
    // Try a simple open without additional UI messaging.
    if (db.open()) return true;

    qDebug() << "❌ ensureOpen(): open failed:" << db.lastError().text();
    return false;
}

QString Connection::lastErrorText() const {
    return db.lastError().text();
}

QStringList Connection::availableDrivers() const {
    return QSqlDatabase::drivers();
}

QString Connection::selectedDriver() const {
    return db.driverName();
}

QString Connection::lastError() const {
    return db.lastError().text();
}
