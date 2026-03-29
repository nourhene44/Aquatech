#include "quotas.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QStringList>
#include <QList>
#include <QDebug>

Quotas::Quotas()
    : idQuota(0), valeurQuota(0.0)
{
}

Quotas::Quotas(int id, const QString &typePoisson, double quotaValue)
    : idQuota(id), typePoisson(typePoisson), valeurQuota(quotaValue)
{
}

// static storage initialization
QString Quotas::m_lastError = QString();
QString Quotas::m_lastQuery = QString();

bool Quotas::ajouter()
{
    QSqlQuery query;
    query.prepare("INSERT INTO QUOTAS (\"TYPE_POISSON\", \"QUOTA\") "
                  "VALUES (:typePoisson, :valeurQuota)");
    query.bindValue(":typePoisson", typePoisson);
    query.bindValue(":valeurQuota", valeurQuota);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        m_lastQuery = query.lastQuery();
        qDebug() << "Quotas::ajouter() failed:" << m_lastError << "query=" << m_lastQuery;
        return false;
    }
    QSqlDatabase::database().commit();
    m_lastError.clear();
    m_lastQuery.clear();
    return true;
}

bool Quotas::supprimer()
{
    QSqlQuery query;
    query.prepare("DELETE FROM QUOTAS WHERE \"ID_QUOTA\" = :idQuota");
    query.bindValue(":idQuota", idQuota);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        m_lastQuery = query.lastQuery();
        qDebug() << "Quotas::supprimer() failed:" << m_lastError << "query=" << m_lastQuery;
        return false;
    }
    QSqlDatabase::database().commit();
    m_lastError.clear();
    m_lastQuery.clear();
    return true;
}

bool Quotas::modifier()
{
    QSqlQuery query;
    query.prepare("UPDATE QUOTAS SET "
                  "\"TYPE_POISSON\" = :typePoisson, "
                  "\"QUOTA\" = :valeurQuota "
                  "WHERE \"ID_QUOTA\" = :idQuota");
    query.bindValue(":typePoisson", typePoisson);
    query.bindValue(":valeurQuota", valeurQuota);
    query.bindValue(":idQuota", idQuota);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        m_lastQuery = query.lastQuery();
        qDebug() << "Quotas::modifier() failed:" << m_lastError << "query=" << m_lastQuery;
        return false;
    }
    QSqlDatabase::database().commit();
    m_lastError.clear();
    m_lastQuery.clear();
    return true;
}

QString Quotas::lastError()
{
    return m_lastError;
}

QString Quotas::lastQuery()
{
    return m_lastQuery;
}

QSqlQueryModel *Quotas::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT * FROM QUOTAS");
    return model;
}

QSqlQueryModel *Quotas::rechercherParId(int id)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery(QString("SELECT * FROM QUOTAS WHERE ID_QUOTA = %1").arg(id));
    return model;
}

// Load all quotas as a list of rows for populating a QTableWidget
QList<QStringList> Quotas::getAllQuotasAsRows()
{
    QList<QStringList> rows;
    QSqlQuery query("SELECT \"ID_QUOTA\", \"TYPE_POISSON\", \"QUOTA\" FROM QUOTAS ORDER BY \"ID_QUOTA\"");
    while (query.next()) {
        QStringList row;
        row << query.value(0).toString()      // ID_QUOTA
            << query.value(1).toString()      // TYPE_POISSON
            << query.value(2).toString();     // QUOTA
        rows.append(row);
    }
    return rows;
}

// Get quotas ordered by fish type for display in table
// Returns only the quota values, assumes rows are in the same order as table
QList<QStringList> Quotas::getQuotasByType()
{
    QList<QStringList> rows;
    // Query to get quotas in a standard order
    QSqlQuery query("SELECT \"TYPE_POISSON\", \"QUOTA\" FROM QUOTAS ORDER BY \"TYPE_POISSON\"");
    while (query.next()) {
        QStringList row;
        row << query.value(0).toString()      // TYPE_POISSON
            << query.value(1).toString();     // QUOTA
        rows.append(row);
    }
    return rows;
}
