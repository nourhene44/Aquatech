#include "bateaauuu.h"
#include "connection.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

static QString g_lastError;

bateaauuu::bateaauuu() {}

namespace {

bool ensureDbOpen(QSqlDatabase* outDb = nullptr)
{
    Connection* conn = Connection::getInstance();
    if (!conn) {
        g_lastError = QStringLiteral("Connexion DB indisponible.");
        return false;
    }

    // Try a silent reopen first; fall back to full createconnect() only if needed.
    if (!conn->ensureOpen()) {
        if (!conn->createconnect()) {
            g_lastError = QStringLiteral("Connexion DB échouée: %1").arg(conn->lastErrorText());
            qWarning() << g_lastError;
            return false;
        }
    }

    QSqlDatabase db = conn->getDatabase();
    if (!db.isValid() || !db.isOpen()) {
        g_lastError = QStringLiteral("Connexion DB indisponible.");
        return false;
    }

    if (outDb) *outDb = db;
    return true;
}

QVariant bindableDate(const QDate& date)
{
    return date.isValid() ? QVariant(date) : QVariant(QVariant::Date);
}

bool execQuery(QSqlQuery& query)
{
    if (query.exec()) {
        g_lastError.clear();
        if (!query.isSelect()) {
            qDebug() << "Query succeeded:" << query.lastQuery();
        }
        return true;
    }

    const QSqlError e = query.lastError();
    g_lastError = QStringLiteral("%1 | driver: %2 | sql: %3")
                      .arg(e.text(), e.driverText(), query.lastQuery());
    qWarning() << "Query failed:" << g_lastError;
    return false;
}

} // namespace

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
        "(ID_BATEAU, NOM, TYPE, CAPACITE, PROPRIETAIRE, STATUT, LARGEUR, PROCHAINE_MAINTENANCE, DATE_ENTREE, DATE_DERNIERE_MAINTENANCE) "
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

QString bateaauuu::genererNouvelId()
{
    QSqlDatabase db;
    if (!ensureDbOpen(&db)) {
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

    if (!execQuery(query)) {
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

    QString finalId = id.trimmed();
    if (finalId.isEmpty()) {
        finalId = genererNouvelId();
        if (finalId.isEmpty()) {
            // g_lastError déjà renseignée
            return false;
        }
    }

    QSqlDatabase db;
    if (!ensureDbOpen(&db)) return false;

    QStringList cols = {
        QStringLiteral("ID_BATEAU"),
        QStringLiteral("NOM"),
        QStringLiteral("TYPE"),
        QStringLiteral("CAPACITE"),
        QStringLiteral("PROPRIETAIRE"),
        QStringLiteral("STATUT"),
        QStringLiteral("LARGEUR"),
        QStringLiteral("PROCHAINE_MAINTENANCE")
    };
    QStringList vals = {
        QStringLiteral(":id"),
        QStringLiteral(":nom"),
        QStringLiteral(":type"),
        QStringLiteral(":capacite"),
        QStringLiteral(":proprietaire"),
        QStringLiteral(":statut"),
        QStringLiteral(":largeur"),
        QStringLiteral(":frequence")
    };

    // Preserve previous behavior: if date is invalid, omit the column so DB defaults can apply.
    if (date_entree.isValid()) {
        cols << QStringLiteral("DATE_ENTREE");
        vals << QStringLiteral(":date_entree");
    }
    if (date_derniere_maintenance.isValid()) {
        cols << QStringLiteral("DATE_DERNIERE_MAINTENANCE");
        vals << QStringLiteral(":date_derniere_maintenance");
    }

    QSqlQuery query(db);
    query.prepare(QStringLiteral("INSERT INTO BATEAUX (%1) VALUES (%2)")
                      .arg(cols.join(QStringLiteral(", ")), vals.join(QStringLiteral(", "))));
    query.bindValue(QStringLiteral(":id"), finalId);
    query.bindValue(QStringLiteral(":nom"), nom);
    query.bindValue(QStringLiteral(":type"), type);
    query.bindValue(QStringLiteral(":capacite"), capacite);
    query.bindValue(QStringLiteral(":proprietaire"), proprietaire);
    query.bindValue(QStringLiteral(":statut"), statut);
    query.bindValue(QStringLiteral(":largeur"), largeur);
    query.bindValue(QStringLiteral(":frequence"), frequence_maintenance);
    if (date_entree.isValid()) {
        query.bindValue(QStringLiteral(":date_entree"), date_entree);
    }
    if (date_derniere_maintenance.isValid()) {
        query.bindValue(QStringLiteral(":date_derniere_maintenance"), date_derniere_maintenance);
    }

    return execQuery(query);
}

bool bateaauuu::updateBateau(const QString& id, const QString& nom, const QString& type,
                             int capacite, const QString& proprietaire, const QString& statut,
                             double largeur, const QDate& date_entree,
                             const QDate& date_derniere_maintenance, int frequence_maintenance) {

    if (id.isEmpty()) return false;

    QSqlDatabase db;
    if (!ensureDbOpen(&db)) return false;

    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "UPDATE BATEAUX SET "
        "NOM = :nom, "
        "TYPE = :type, "
        "CAPACITE = :capacite, "
        "PROPRIETAIRE = :proprietaire, "
        "STATUT = :statut, "
        "LARGEUR = :largeur, "
        "DATE_ENTREE = :date_entree, "
        "DATE_DERNIERE_MAINTENANCE = :date_derniere_maintenance, "
        "PROCHAINE_MAINTENANCE = :frequence "
        "WHERE ID_BATEAU = :id"));

    query.bindValue(QStringLiteral(":nom"), nom);
    query.bindValue(QStringLiteral(":type"), type);
    query.bindValue(QStringLiteral(":capacite"), capacite);
    query.bindValue(QStringLiteral(":proprietaire"), proprietaire);
    query.bindValue(QStringLiteral(":statut"), statut);
    query.bindValue(QStringLiteral(":largeur"), largeur);
    query.bindValue(QStringLiteral(":date_entree"), bindableDate(date_entree));
    query.bindValue(QStringLiteral(":date_derniere_maintenance"), bindableDate(date_derniere_maintenance));
    query.bindValue(QStringLiteral(":frequence"), frequence_maintenance);
    query.bindValue(QStringLiteral(":id"), id.trimmed());

    return execQuery(query);
}

bool bateaauuu::deleteBateau(const QString& id) {
    if (id.isEmpty()) return false;

    QSqlDatabase db;
    if (!ensureDbOpen(&db)) return false;

    bool ok = false;
    const int idNum = id.trimmed().toInt(&ok);
    if (ok && idNum == 0) {
        g_lastError = QStringLiteral("Le bateau systeme ID 0 ne peut pas etre supprime.");
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

