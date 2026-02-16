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
        QString nomSourceDonnees = "AQUATEC";  // Nom de votre source ODBC
        QString nomUtilisateur = "nour1";            // Nom d'utilisateur Oracle
        QString motDePasse = "nour123";              // Mot de passe Oracle

        // Configuration de la connexion
        db.setDatabaseName(nomSourceDonnees);  // ← Correction: plus de variable 'source'
        db.setUserName(nomUtilisateur);        // ← Correction: plus de variable 'hr'
        db.setPassword(motDePasse);            // ← Correction: plus de variable 'hr'
        db.setHostName("localhost");
        db.setPort(1521);  // Port par défaut d'Oracle

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
        qDebug() << "📌 Nom de la source : " << nomSourceDonnees;
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
