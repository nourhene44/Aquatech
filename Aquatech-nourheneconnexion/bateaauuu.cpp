#include "bateaauuu.h"
#include "connection.h"
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

static bool checkConnection() {
    Connection conn;
    if (!conn.createconnect()) {
        g_lastError = "DB connect failed";
        qWarning() << g_lastError;
        return false;
    }
    return true;
}

bool bateaauuu::addBateau(const QString& id, const QString& nom, const QString& type,
                          int capacite, const QString& proprietaire, const QString& statut,
                          double largeur, const QDate& date_entree,
                          const QDate& date_derniere_maintenance, int frequence_maintenance) {

    if (!checkConnection()) return false;

    QString finalId = id.isEmpty()
                          ? QString::number(QDate::currentDate().toJulianDay()) + QString::number(QTime::currentTime().msec())
                          : id;

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

    QString sql = QString("DELETE FROM BATEAUX WHERE ID_BATEAU = %1")
                      .arg(formatId(id));

    return executeQuery(sql);
}

QString bateaauuu::lastError() {
    return g_lastError;
}
