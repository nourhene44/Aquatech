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
    // Oracle ODBC: avoid binding NULL QVariant for NUMBER columns (can be treated as BINARY).
    if (idBateau == -1) {
        query.prepare("INSERT INTO captures (ID_CAPTURE, ID_BATEAU, TYPE_POISSON, QUANTITE, POIDS, DATE_CAPTURE) "
                      "VALUES (:idCapture, NULL, :type, :quantite, :poids, :dateCapture)");
    } else {
        query.prepare("INSERT INTO captures (ID_CAPTURE, ID_BATEAU, TYPE_POISSON, QUANTITE, POIDS, DATE_CAPTURE) "
                      "VALUES (:idCapture, :idBateau, :type, :quantite, :poids, :dateCapture)");
        query.bindValue(":idBateau", idBateau);
    }
    query.bindValue(":idCapture", idCapture);
    query.bindValue(":type", typePoisson);
    query.bindValue(":quantite", quantite);
    query.bindValue(":poids", poids);
    // bind QDate as QVariant (driver will send DATE)
    query.bindValue(":dateCapture", QVariant(dateCapture));

    qDebug() << "ajouter() bind check: idCapture=" << idCapture
             << "idBateau=" << idBateau
             << "quantite=" << quantite
             << "poids=" << poids
             << "dateCapture=" << dateCapture;

    if (idBateau != -1) {
        const QVariant v = query.boundValue(QStringLiteral(":idBateau"));
        qDebug() << "ajouter() :idBateau bound typeId=" << v.typeId()
                 << "typeName=" << (v.metaType().name() ? v.metaType().name() : "<null>")
                 << "isNull=" << v.isNull()
                 << "value=" << v.toString();
    }

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
    // Oracle ODBC: avoid binding NULL QVariant for NUMBER columns (can be treated as BINARY).
    if (idBateau > 0) {
        query.prepare("UPDATE captures SET "
                      "ID_BATEAU = :idBateau, "
                      "TYPE_POISSON = :type, "
                      "QUANTITE = :quantite, "
                      "POIDS = :poids, "
                      "DATE_CAPTURE = :dateCapture "
                      "WHERE ID_CAPTURE = :idCapture");
        query.bindValue(":idBateau", idBateau);
    } else {
        query.prepare("UPDATE captures SET "
                      "ID_BATEAU = NULL, "
                      "TYPE_POISSON = :type, "
                      "QUANTITE = :quantite, "
                      "POIDS = :poids, "
                      "DATE_CAPTURE = :dateCapture "
                      "WHERE ID_CAPTURE = :idCapture");
    }
    query.bindValue(":type", typePoisson);
    query.bindValue(":quantite", quantite);
    query.bindValue(":poids", poids);
    // bind QDate as QVariant (driver will send DATE)
    query.bindValue(":dateCapture", QVariant(dateCapture));
    query.bindValue(":idCapture", idCapture);

    qDebug() << "modifier() bind check: idCapture=" << idCapture
             << "idBateau=" << idBateau
             << "quantite=" << quantite
             << "poids=" << poids
             << "dateCapture=" << dateCapture;

    if (idBateau > 0) {
        const QVariant v = query.boundValue(QStringLiteral(":idBateau"));
        qDebug() << "modifier() :idBateau bound typeId=" << v.typeId()
                 << "typeName=" << (v.metaType().name() ? v.metaType().name() : "<null>")
                 << "isNull=" << v.isNull()
                 << "value=" << v.toString();
    }
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
