#ifndef QUOTAS_H
#define QUOTAS_H

#include <QString>
#include <QSqlQueryModel>
#include <QList>

// Helper class for managing QUOTAS table in the database
// Each row represents a quota for a specific fish type
class Quotas
{
public:
    Quotas();
    Quotas(int id, const QString &typePoisson, double quotaValue);

    // Data members representing a row in the database
    int idQuota;
    QString typePoisson;
    double valeurQuota;  // quota value (in kg or units)

    // CRUD operations
    bool ajouter();              // create
    bool supprimer();            // delete (uses idQuota field)
    bool modifier();             // update (uses idQuota field)

    // Static methods for retrieving data
    static QString lastError();
    static QString lastQuery();
    static QSqlQueryModel *afficher();                      // read all
    static QSqlQueryModel *rechercherParId(int idQuota);    // read one row
    static QList<QStringList> getAllQuotasAsRows();         // read all as list of string lists
    static QList<QStringList> getQuotasByType();            // get quotas ordered by fish type

private:
    static QString m_lastError;
    static QString m_lastQuery;
};

#endif // QUOTAS_H
