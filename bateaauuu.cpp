#include "bateaauuu.h"
#include "connection.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

static QString g_lastError;

bateaauuu::bateaauuu() {}

static QString escapeString(const QString& str) {
    QString escaped = str;
    return escaped.replace("'", "''");
}

static QString formatId(const QString& id) {
    bool isNum;
    id.toLongLong(&isNum);
    return isNum ? id : "'" + escapeString(id) + "'";
}

static QString formatDate(const QDate& date) {
    return date.isValid()
    ? QString("TO_DATE('%1','YYYY-MM-DD')").arg(date.toString("yyyy-MM-dd"))
    : "NULL";
}

static QString buildSetClause(const QString& nom, const QString& type, int capacite,
                              const QString& proprietaire, const QString& statut,
                              double largeur, const QDate& date_entree,
                              const QDate& date_derniere_maintenance, int frequence_maintenance) {
    QStringList set;
    set << QString("NOM = '%1'").arg(escapeString(nom));
    set << QString("TYPE = '%1'").arg(escapeString(type));
    set << QString("CAPACITE = %1").arg(capacite);
    set << QString("PROPRIETAIRE = '%1'").arg(escapeString(proprietaire));
    set << QString("STATUT = '%1'").arg(escapeString(statut));
    set << QString("LARGEUR = %1").arg(largeur);
    set << QString("DATE_ENTREE = %1").arg(formatDate(date_entree));
    set << QString("DATE_DERNIERE_MAINTENANCE = %1").arg(formatDate(date_derniere_maintenance));
    set << QString("FREQUENCE_MAINTENANCE = %1").arg(frequence_maintenance);
    return set.join(", ");
}

static bool executeQuery(const QString& sql) {
    QSqlQuery query;
    if (query.exec(sql)) {
        g_lastError.clear();
        qDebug() << "Query succeeded:" << sql;
        return true;
    }

    QSqlError e = query.lastError();
    g_lastError = QString("%1 | driver: %2 | sql: %3").arg(e.text(), e.driverText(), sql);
    qWarning() << "Query failed:" << g_lastError;
    return false;
}

static QString friendlyDeleteError(const QString& raw)
{
    const QString msg = raw.trimmed();
    if (msg.contains("ORA-02292", Qt::CaseInsensitive)) {
        return QStringLiteral("Suppression impossible: ce bateau est encore reference dans des enregistrements enfants.");
    }
    if (msg.contains("ORA-02291", Qt::CaseInsensitive)) {
        return QStringLiteral("Suppression impossible: la valeur 0 n'est pas autorisee pour ID_BATEAU (contrainte FK).");
    }
    return msg;
}

static bool ensureZeroBoatExists(QSqlDatabase& db)
{
    QSqlQuery check(db);
    if (!check.exec("SELECT COUNT(*) FROM BATEAUX WHERE ID_BATEAU = 0") || !check.next()) {
        g_lastError = check.lastError().text();
        return false;
    }

    if (check.value(0).toInt() > 0) {
        return true;
    }

    QString defaultType = QStringLiteral("Type inconnu");
    QString defaultStatut = QStringLiteral("Inactif");

    QSqlQuery seed(db);
    if (seed.exec("SELECT TYPE, STATUT FROM BATEAUX WHERE ROWNUM = 1") && seed.next()) {
        const QString t = seed.value(0).toString().trimmed();
        const QString s = seed.value(1).toString().trimmed();
        if (!t.isEmpty()) defaultType = t;
        if (!s.isEmpty()) defaultStatut = s;
    }

    QSqlQuery ins(db);
    ins.prepare(
        "INSERT INTO BATEAUX "
        "(ID_BATEAU, NOM, TYPE, CAPACITE, PROPRIETAIRE, STATUT, LARGEUR, FREQUENCE_MAINTENANCE, DATE_ENTREE, DATE_DERNIERE_MAINTENANCE) "
        "VALUES (0, :nom, :type, 0, :prop, :statut, 0, 0, NULL, NULL)");
    ins.bindValue(":nom", QStringLiteral("AUCUN BATEAU"));
    ins.bindValue(":type", defaultType);
    ins.bindValue(":prop", QStringLiteral("SYSTEME"));
    ins.bindValue(":statut", defaultStatut);
    if (!ins.exec()) {
        g_lastError = ins.lastError().text();
        return false;
    }

    return true;
}

static bool checkConnection() {
    Connection* conn = Connection::getInstance();
    if (!conn->createconnect()) {
        g_lastError = "DB connect failed";
        qWarning() << g_lastError;
        return false;
    }
    return true;
}

QString bateaauuu::genererNouvelId()
{
    Connection* conn = Connection::getInstance();
    if (!conn->ensureOpen()) {
        g_lastError = QStringLiteral("Connexion DB échouée: %1").arg(conn->lastErrorText());
        return QString();
    }

    QSqlDatabase db = conn->getDatabase();
    if (!db.isValid() || !db.isOpen()) {
        g_lastError = QStringLiteral("Connexion DB indisponible.");
        return QString();
    }

    // Schéma demandé: 261NNN
    const int prefix = 261;
    const int minId = prefix * 1000;
    const int maxId = minId + 999;

    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT NVL(MAX(TO_NUMBER(ID_BATEAU)), 0) FROM BATEAUX "
        "WHERE ID_BATEAU IS NOT NULL "
        "AND REGEXP_LIKE(TRIM(TO_CHAR(ID_BATEAU)), '^[0-9]+$') "
        "AND TO_NUMBER(ID_BATEAU) BETWEEN :minId AND :maxId"));
    query.bindValue(QStringLiteral(":minId"), minId);
    query.bindValue(QStringLiteral(":maxId"), maxId);

    if (!query.exec()) {
        g_lastError = query.lastError().text();
        return QString();
    }

    int currentMax = 0;
    if (query.next()) {
        currentMax = query.value(0).toInt();
    }

    const int nextId = (currentMax > 0) ? (currentMax + 1) : (minId + 1);
    if (nextId > maxId) {
        g_lastError = QStringLiteral("Limite atteinte pour ce préfixe ID (261NNN).");
        return QString();
    }

    g_lastError.clear();
    return QString::number(nextId);
}

bool bateaauuu::addBateau(const QString& id, const QString& nom, const QString& type,
                          int capacite, const QString& proprietaire, const QString& statut,
                          double largeur, const QDate& date_entree,
                          const QDate& date_derniere_maintenance, int frequence_maintenance) {

    if (!checkConnection()) return false;

    QString finalId = id.trimmed();
    if (finalId.isEmpty()) {
        finalId = genererNouvelId();
        if (finalId.isEmpty()) {
            // g_lastError déjà renseignée
            return false;
        }
    }

    QStringList cols = {"ID_BATEAU", "NOM", "TYPE", "CAPACITE", "PROPRIETAIRE",
                        "STATUT", "LARGEUR", "FREQUENCE_MAINTENANCE"};
    QStringList vals = {formatId(finalId),
                        "'" + escapeString(nom) + "'",
                        "'" + escapeString(type) + "'",
                        QString::number(capacite),
                        "'" + escapeString(proprietaire) + "'",
                        "'" + escapeString(statut) + "'",
                        QString::number(largeur),
                        QString::number(frequence_maintenance)};

    if (date_entree.isValid()) {
        cols << "DATE_ENTREE";
        vals << formatDate(date_entree);
    }
    if (date_derniere_maintenance.isValid()) {
        cols << "DATE_DERNIERE_MAINTENANCE";
        vals << formatDate(date_derniere_maintenance);
    }

    QString sql = QString("INSERT INTO BATEAUX (%1) VALUES (%2)")
                      .arg(cols.join(", "), vals.join(", "));

    return executeQuery(sql);
}

bool bateaauuu::updateBateau(const QString& id, const QString& nom, const QString& type,
                             int capacite, const QString& proprietaire, const QString& statut,
                             double largeur, const QDate& date_entree,
                             const QDate& date_derniere_maintenance, int frequence_maintenance) {

    if (id.isEmpty() || !checkConnection()) return false;

    QString setClause = buildSetClause(nom, type, capacite, proprietaire, statut,
                                       largeur, date_entree, date_derniere_maintenance,
                                       frequence_maintenance);

    QString sql = QString("UPDATE BATEAUX SET %1 WHERE ID_BATEAU = %2")
                      .arg(setClause, formatId(id));

    return executeQuery(sql);
}

bool bateaauuu::deleteBateau(const QString& id) {
    if (id.isEmpty() || !checkConnection()) return false;

    bool ok = false;
    const int idNum = id.trimmed().toInt(&ok);
    if (ok && idNum == 0) {
        g_lastError = QStringLiteral("Le bateau systeme ID 0 ne peut pas etre supprime.");
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid()) {
        g_lastError = QStringLiteral("Base de donnees indisponible.");
        return false;
    }

    if (!db.transaction()) {
        g_lastError = QStringLiteral("Impossible de demarrer la transaction de suppression.");
        return false;
    }

    if (!ensureZeroBoatExists(db)) {
        db.rollback();
        return false;
    }

    QSqlQuery upd(db);
    upd.prepare("UPDATE PECHEURS SET ID_BATEAU = 0 WHERE ID_BATEAU = :id");
    upd.bindValue(":id", id.trimmed());
    if (!upd.exec()) {
        db.rollback();
        g_lastError = friendlyDeleteError(upd.lastError().text());
        return false;
    }

    QSqlQuery del(db);
    del.prepare("DELETE FROM BATEAUX WHERE ID_BATEAU = :id");
    del.bindValue(":id", id.trimmed());
    if (!del.exec()) {
        db.rollback();
        g_lastError = friendlyDeleteError(del.lastError().text());
        return false;
    }

    if (!db.commit()) {
        db.rollback();
        g_lastError = QStringLiteral("Suppression annulee: echec commit transaction.");
        return false;
    }

    g_lastError.clear();
    return true;
}

QString bateaauuu::lastError() {
    return g_lastError;
}

