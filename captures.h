#ifndef CAPTURES_H
#define CAPTURES_H

#include <QString>
#include <QDate>
#include <QSqlQueryModel>

// This helper class encapsulates the basic CRUD operations for the
// "captures" table in the database.  The column names used in the
// queries (idCapture, idBateau, typePoisson, quantite, poids, date)
// must match whatever schema you have created on the ODBC data source
// named "Taher".  Adjust them if your actual table uses different
// identifiers (for example upper‑case names, singular/plural, etc.).
//
// Usage example from a slot in MainWindow:
//
//   Captures c(idVal, boatVal, typeVal, qtyVal, weightVal, dateVal);
//   if (!c.ajouter()) {
//       QMessageBox::warning(this, "Error", "Could not insert capture");
//   }
//
// and to display in a QTableView:
//
//   ui->tableView->setModel(Captures::afficher());
//
class Captures
{
public:
    Captures();
    
    // getters/setters could be added if needed
    Captures(int idCapture,
             int idBateau,
             const QString &typePoisson,
             int quantite,
             double poids,
             const QDate &dateCapture);

    // data members representing a row in the database
    int idCapture;
    int idBateau;
    QString typePoisson;
    int quantite;
    double poids;
    QDate dateCapture;

    // CRUD operations (instance methods operate on the contents of this object)
    bool ajouter();              // create
    bool supprimer();            // delete (uses idCapture field)
    bool modifier();             // update (uses idCapture field)

    // retrieve description of last error from any CRUD call
    static QString lastError();
    static QString lastQuery();


    // helpers that do not require an object instance
    static QSqlQueryModel *afficher();                        // read all
    static QSqlQueryModel *rechercherParId(int idCapture);    // read one row
    static QList<QStringList> getAllCapturesAsRows();         // read all as list of string lists for QTableWidget

private:
    // storage for the last error text produced by ajout/modif/supp.
    static QString m_lastError;
    static QString m_lastQuery;
};

#endif // CAPTURES_H
