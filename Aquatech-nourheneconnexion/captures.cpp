#include "captures.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QStringList>
#include <QList>

Captures::Captures()
    : idCapture(0), idBateau(0), quantite(0), poids(0.0), dateCapture(QDate::currentDate())
{
}

Captures::Captures(int idCapture,
                   int idBateau,
                   const QString &typePoisson,
                   int quantite,
                   double poids,
                   const QDate &dateCapture)
    : idCapture(idCapture),
      idBateau(idBateau),
      typePoisson(typePoisson),
      quantite(quantite),
      poids(poids),
      dateCapture(dateCapture)
{
}

// static storage initialization
QString Captures::m_lastError = QString();
QString Captures::m_lastQuery = QString();

bool Captures::ajouter()
{
    QSqlQuery query;
    // column names must match the actual schema; TYPE_POISSON contains an underscore
    // identifiers must match column names exactly as seen in the database
    query.prepare("INSERT INTO captures (ID_CAPTURE, ID_BATEAU, TYPE_POISSON, QUANTITE, POIDS, DATE_CAPTURE) "
                  "VALUES (:idCapture, :idBateau, :type, :quantite, :poids, :dateCapture)");
    query.bindValue(":idCapture", idCapture);
    // use NULL for -1 to bypass FK constraint during testing
    if (idBateau == -1) {
        // bind NULL value for NUMBER column
        query.bindValue(":idBateau", QVariant());
    } else {
        query.bindValue(":idBateau", idBateau);
    }
    query.bindValue(":type", typePoisson);
    query.bindValue(":quantite", quantite);
    query.bindValue(":poids", poids);
    // bind QDate as QVariant (driver will send DATE)
    query.bindValue(":dateCapture", QVariant(dateCapture));

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        m_lastQuery = query.lastQuery();
        qDebug() << "ajouter() failed:" << m_lastError << "query=" << m_lastQuery;
        return false;
    }
    // commit the transaction to persist data to database
    QSqlDatabase::database().commit();
    m_lastError.clear();
    m_lastQuery.clear();
    return true;
}

bool Captures::supprimer()
{
    QSqlQuery query;
    query.prepare("DELETE FROM captures WHERE ID_CAPTURE = :idCapture");
    query.bindValue(":idCapture", idCapture);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        m_lastQuery = query.lastQuery();
        qDebug() << "supprimer() failed:" << m_lastError << "query=" << m_lastQuery;
        return false;
    }
    // commit the transaction to persist data to database
    QSqlDatabase::database().commit();
    m_lastError.clear();
    m_lastQuery.clear();
    return true;
}

bool Captures::modifier()
{
    // ID_BATEAU can be NULL; only validate FK existence when a non-null value is provided.
    if (idBateau > 0) {
        QSqlQuery boatExistsQuery;
        const QString fkCheckSql = QStringLiteral("SELECT COUNT(1) FROM HR.BATEAUX WHERE ID_BATEAU = :idBateau");
        boatExistsQuery.prepare(fkCheckSql);
        boatExistsQuery.bindValue(":idBateau", idBateau);
        if (!boatExistsQuery.exec() || !boatExistsQuery.next()) {
            m_lastError = boatExistsQuery.lastError().text();
            m_lastQuery = fkCheckSql;
            qDebug() << "modifier() FK check failed:" << m_lastError << "query=" << m_lastQuery;
            return false;
        }

        if (boatExistsQuery.value(0).toInt() <= 0) {
            m_lastError = QStringLiteral("ID_BATEAU inexistant dans BATEAUX: %1").arg(idBateau);
            m_lastQuery = fkCheckSql;
            qDebug() << "modifier() FK check failed:" << m_lastError << "query=" << m_lastQuery;
            return false;
        }
    }

    QSqlQuery query;
    // update the DATE_CAPTURE column and other fields
    // make sure the fish type column name matches the table
    query.prepare("UPDATE captures SET "
                  "ID_BATEAU = :idBateau, "
                  "TYPE_POISSON = :type, "
                  "QUANTITE = :quantite, "
                  "POIDS = :poids, "
                  "DATE_CAPTURE = :dateCapture "
                  "WHERE ID_CAPTURE = :idCapture");
    if (idBateau > 0) {
        query.bindValue(":idBateau", idBateau);
    } else {
        query.bindValue(":idBateau", QVariant());
    }
    query.bindValue(":type", typePoisson);
    query.bindValue(":quantite", quantite);
    query.bindValue(":poids", poids);
    // bind QDate as QVariant (driver will send DATE)
    query.bindValue(":dateCapture", QVariant(dateCapture));
    query.bindValue(":idCapture", idCapture);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        m_lastQuery = query.lastQuery();
        qDebug() << "modifier() failed:" << m_lastError << "query=" << m_lastQuery;
        return false;
    }
    // commit the transaction to persist data to database
    QSqlDatabase::database().commit();
    m_lastError.clear();
    m_lastQuery.clear();
    return true;
}

QString Captures::lastError()
{
    return m_lastError;
}

QString Captures::lastQuery()
{
    return m_lastQuery;
}


QSqlQueryModel *Captures::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT * FROM captures"); // SELECT * is case‑insensitive for column names
    return model;
}

QSqlQueryModel *Captures::rechercherParId(int id)
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery(QString("SELECT * FROM captures WHERE ID_CAPTURE = %1").arg(id));
    return model;
}

// Load all captures as a list of rows for populating a QTableWidget
QList<QStringList> Captures::getAllCapturesAsRows()
{
    QList<QStringList> rows;
    m_lastError.clear();
    m_lastQuery.clear();

    QSqlQuery query;
    const QString sqlHr = QStringLiteral("SELECT ID_CAPTURE, ID_BATEAU, TYPE_POISSON, QUANTITE, POIDS, DATE_CAPTURE FROM HR.CAPTURES ORDER BY ID_CAPTURE DESC");
    const QString sqlDefault = QStringLiteral("SELECT ID_CAPTURE, ID_BATEAU, TYPE_POISSON, QUANTITE, POIDS, DATE_CAPTURE FROM CAPTURES ORDER BY ID_CAPTURE DESC");

    bool ok = false;
    query.prepare(sqlHr);
    if (query.exec()) {
        ok = true;
        m_lastQuery = sqlHr;
    } else {
        qDebug() << "getAllCapturesAsRows HR.CAPTURES failed:" << query.lastError().text() << "query=" << sqlHr;
        query.prepare(sqlDefault);
        if (query.exec()) {
            ok = true;
            m_lastQuery = sqlDefault;
        }
    }

    if (!ok) {
        m_lastError = query.lastError().text();
        m_lastQuery = sqlDefault;
        qDebug() << "getAllCapturesAsRows failed:" << m_lastError << "query=" << m_lastQuery;
        return rows;
    }

    while (query.next()) {
        QStringList row;
        row << query.value(0).toString()      // ID_CAPTURE
            << (query.value(1).isNull() ? QStringLiteral("") : query.value(1).toString())  // ID_BATEAU (NULL safe)
            << query.value(2).toString()      // TYPE_POISSON
            << query.value(3).toString()      // QUANTITE
            << query.value(4).toString()      // POIDS
            << query.value(5).toString();     // DATE_CAPTURE
        rows.append(row);
    }
    return rows;
}
