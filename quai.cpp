#include "quai.h"
#include "connection.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

Quai::Quai() {}

int Quai::genererNouvelId()
{
    Connection *conn = Connection::getInstance();
    if (!conn->ensureOpen()) {
        m_lastError = QStringLiteral("Connexion DB échouée: %1").arg(conn->lastErrorText());
        return 0;
    }

    QSqlDatabase db = conn->getDatabase();
    if (!db.isValid() || !db.isOpen()) {
        m_lastError = QStringLiteral("Base de données non ouverte");
        return 0;
    }

    const int prefix = 263;
    const int minId = prefix * 10000;
    const int maxId = minId + 9999;

    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT NVL(MAX(idnum), 0) FROM ("
        "  SELECT TO_NUMBER(REGEXP_SUBSTR(TRIM(TO_CHAR(ID_QUAI)), '^[0-9]+$')) AS idnum "
        "  FROM QUAIS"
        ") WHERE idnum BETWEEN :minId AND :maxId"));
    query.bindValue(QStringLiteral(":minId"), minId);
    query.bindValue(QStringLiteral(":maxId"), maxId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return 0;
    }

    int currentMax = 0;
    if (query.next()) {
        currentMax = query.value(0).toInt();
    }

    const int nextId = (currentMax > 0) ? (currentMax + 1) : (minId + 1);
    if (nextId > maxId) {
        m_lastError = QStringLiteral("Limite atteinte pour ce préfixe ID (263NNNN).");
        return 0;
    }

    m_lastError.clear();
    return nextId;
}

Quai::Quai(int idQuai,
           const QString &nomQuai,
           const QString &zonePort,
           const QString &zoneCouverte,
           double longueur,
           int capaciteQuais,
           const QString &statutQuai)
    : m_idQuai(idQuai)
    , m_nomQuai(nomQuai)
    , m_zonePort(zonePort)
    , m_zoneCouverte(zoneCouverte)
    , m_longueur(longueur)
    , m_capaciteQuais(capaciteQuais)
    , m_statut(statutQuai)
{
}

// Getters / setters

int Quai::idQuai() const
{
    return m_idQuai;
}

void Quai::setIdQuai(int id)
{
    m_idQuai = id;
}

QString Quai::nomQuai() const
{
    return m_nomQuai;
}

void Quai::setNomQuai(const QString &nom)
{
    m_nomQuai = nom;
}

QString Quai::zonePort() const
{
    return m_zonePort;
}

void Quai::setZonePort(const QString &zone)
{
    m_zonePort = zone;
}

QString Quai::zoneCouverte() const
{
    return m_zoneCouverte;
}

void Quai::setZoneCouverte(const QString &zone)
{
    m_zoneCouverte = zone;
}

double Quai::longueur() const
{
    return m_longueur;
}

void Quai::setLongueur(double valeur)
{
    m_longueur = valeur;
}

int Quai::capaciteQuais() const
{
    return m_capaciteQuais;
}

void Quai::setCapaciteQuais(int capacite)
{
    m_capaciteQuais = capacite;
}

QString Quai::statut() const
{
    return m_statut;
}

void Quai::setStatut(const QString &statut)
{
    m_statut = statut;
}

// CRUD style atelier, basés sur les attributs de l'objet

bool Quai::ajouter()
{
    QVariantMap donnees;
    donnees["ID_QUAI"] = m_idQuai;
    donnees["NOM_QUAI"] = m_nomQuai;
    donnees["ZONE_PORT"] = m_zonePort;
    donnees["ZONE_COUVERTE"] = m_zoneCouverte;
    donnees["LONGUEUR"] = m_longueur;
    donnees["CAPACITE_QUAIS"] = m_capaciteQuais;
    donnees["STATUT"] = m_statut;

    return ajouter(donnees);
}

bool Quai::modifier()
{
    if (m_idQuai <= 0)
        return false;

    QVariantMap donnees;
    donnees["ID_QUAI"] = m_idQuai;
    donnees["NOM_QUAI"] = m_nomQuai;
    donnees["ZONE_PORT"] = m_zonePort;
    donnees["ZONE_COUVERTE"] = m_zoneCouverte;
    donnees["LONGUEUR"] = m_longueur;
    donnees["CAPACITE_QUAIS"] = m_capaciteQuais;
    donnees["STATUT"] = m_statut;

    return modifier(m_idQuai, donnees);
}

bool Quai::supprimer()
{
    if (m_idQuai <= 0)
        return false;

    return supprimer(m_idQuai);
}

bool Quai::ajouter(const QVariantMap &donnees)
{
    QSqlDatabase db = Connection::getInstance()->getDatabase();
    if (!db.isOpen()) {
        m_lastError = QStringLiteral("Base de données non ouverte");
        qDebug() << "Base de données non ouverte";
        return false;
    }

    int idQuai = donnees.value("ID_QUAI").toInt();
    if (idQuai <= 0) {
        idQuai = genererNouvelId();
        if (idQuai <= 0) {
            if (m_lastError.trimmed().isEmpty()) {
                m_lastError = QStringLiteral("Impossible de générer un nouvel ID quai.");
            }
            return false;
        }
    }

    // ID_BATEAU optionnel :
    // - 0 ou valeur manquante  => NULL en base (aucun bateau)
    // - > 0                    => cl├⌐ existante dans BATEAUX
    QVariant idBateauVar;
    if (donnees.contains("ID_BATEAU")) {
        const int idBateau = donnees.value("ID_BATEAU").toInt();
        if (idBateau > 0) {
            idBateauVar = idBateau;
        } else {
            // force un NULL pour ne pas violer la contrainte FK_QUAI_BATEAU
            idBateauVar = QVariant(QVariant::Int);
        }
    } else {
        idBateauVar = QVariant(QVariant::Int);
    }

    QSqlQuery query(db);
    // Utilisation de paramètres positionnels "?" comme dans les autres insertions du projet,
    // pour éviter les problèmes du driver ODBC avec les noms de paramètres.
    query.prepare("INSERT INTO QUAIS (ID_QUAI, NOM_QUAI, ZONE_PORT, ZONE_COUVERTE, LONGUEUR, CAPACITE_QUAIS, STATUT, ID_BATEAU) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");

    query.addBindValue(idQuai);
    query.addBindValue(donnees.value("NOM_QUAI").toString());
    query.addBindValue(donnees.value("ZONE_PORT").toString());
    query.addBindValue(donnees.value("ZONE_COUVERTE").toString());
    query.addBindValue(donnees.value("LONGUEUR").toDouble());
    query.addBindValue(donnees.value("CAPACITE_QUAIS").toInt());
    query.addBindValue(donnees.value("STATUT").toString());
    query.addBindValue(idBateauVar);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qDebug() << "Erreur ajout quai:" << m_lastError;
        return false;
    }
    m_lastError.clear();
    return true;
}

QList<QVariantMap> Quai::lister()
{
    QList<QVariantMap> liste;
    QSqlDatabase db = Connection::getInstance()->getDatabase();
    if (!db.isOpen()) {
        m_lastError = QStringLiteral("Base de données non ouverte");
        return liste;
    }

    QSqlQuery query(db);
    query.setForwardOnly(true);
    if (!query.exec("SELECT ID_QUAI, NOM_QUAI, ZONE_PORT, ZONE_COUVERTE, LONGUEUR, CAPACITE_QUAIS, STATUT, ID_BATEAU FROM QUAIS")) {
        m_lastError = query.lastError().text();
        qDebug() << "Erreur affichage quais:" << m_lastError;
        return liste;
    }

    while (query.next()) {
        QVariantMap ligne;
        ligne["ID_QUAI"] = query.value(0);
        ligne["NOM_QUAI"] = query.value(1);
        ligne["ZONE_PORT"] = query.value(2);
        ligne["ZONE_COUVERTE"] = query.value(3);
        ligne["LONGUEUR"] = query.value(4);
        ligne["CAPACITE_QUAIS"] = query.value(5);
        ligne["STATUT"] = query.value(6);
        ligne["ID_BATEAU"] = query.value(7);
        liste.append(ligne);
    }
    return liste;
}

bool Quai::modifier(int id, const QVariantMap &donnees)
{
    QSqlDatabase db = Connection::getInstance()->getDatabase();
    if (!db.isOpen() || donnees.isEmpty()) {
        m_lastError = !db.isOpen()
                ? QStringLiteral("Base de données non ouverte")
                : QStringLiteral("Aucune donnée fournie pour la mise à jour");
        return false;
    }

    QStringList sets;
    QVariantList values;
    for (auto it = donnees.begin(); it != donnees.end(); ++it) {
        QString col = it.key();
        QVariant val = it.value();

        if (col.compare(QStringLiteral("ID_BATEAU"), Qt::CaseInsensitive) == 0) {
            // M├¬me logique que pour l'ajout : 0 ou valeur vide => NULL (aucun bateau)
            int idBateau = val.toInt();
            if (idBateau <= 0) {
                val = QVariant(QVariant::Int);
            }
        }

        sets << QString("%1 = ?").arg(col);
        values << val;
    }
    QString sql = QString("UPDATE QUAIS SET %1 WHERE ID_QUAI = ?").arg(sets.join(", "));
    QSqlQuery query(db);
    query.prepare(sql);
    for (const auto &v : values)
        query.addBindValue(v);
    query.addBindValue(id);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qDebug() << "Erreur modification quai:" << m_lastError;
        return false;
    }
    m_lastError.clear();
    return true;
}

bool Quai::supprimer(int id)
{
    QSqlDatabase db = Connection::getInstance()->getDatabase();
    if (!db.isOpen()) {
        m_lastError = QStringLiteral("Base de données non ouverte");
        return false;
    }

    QSqlQuery query(db);
    query.prepare("DELETE FROM QUAIS WHERE ID_QUAI = ?");
    query.addBindValue(id);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qDebug() << "Erreur suppression quai:" << m_lastError;
        return false;
    }
    m_lastError.clear();
    return true;
}

QVariantMap Quai::chargerInfos(int id)
{
    QVariantMap infos;
    QSqlDatabase db = Connection::getInstance()->getDatabase();
    if (!db.isOpen()) {
        m_lastError = QStringLiteral("Base de données non ouverte");
        return infos;
    }

    QSqlQuery query(db);
    query.prepare("SELECT ID_QUAI, NOM_QUAI, ZONE_PORT, ZONE_COUVERTE, LONGUEUR, CAPACITE_QUAIS, STATUT, ID_BATEAU FROM QUAIS WHERE ID_QUAI = ?");
    query.addBindValue(id);
    if (query.exec() && query.next()) {
        infos["ID_QUAI"] = query.value(0);
        infos["NOM_QUAI"] = query.value(1);
        infos["ZONE_PORT"] = query.value(2);
        infos["ZONE_COUVERTE"] = query.value(3);
        infos["LONGUEUR"] = query.value(4);
        infos["CAPACITE_QUAIS"] = query.value(5);
        infos["STATUT"] = query.value(6);
        infos["ID_BATEAU"] = query.value(7);
    } else if (query.lastError().isValid()) {
        m_lastError = query.lastError().text();
    }
    return infos;
}

void Quai::setModeModification(bool mode, int id)
{
    m_modeModification = mode;
    m_idEnCours = mode ? id : -1;
    if (mode && id != -1) {
        m_donneesEnCours = chargerInfos(id);
    } else {
        m_donneesEnCours.clear();
    }
}

bool Quai::isModeModification() const
{
    return m_modeModification;
}

int Quai::idEnCours() const
{
    return m_idEnCours;
}