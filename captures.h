#ifndef CAPTURES_H
#define CAPTURES_H

#include <QString>
#include <QDate>
#include <QVector>
#include <QSqlQueryModel>

class captures
{
public:
    captures();
    captures(const QString& idCapture,
             int idBateau,
             const QString& typePoisson,
             int quantite,
             double poids,
             const QDate& dateCapture);

    struct TableRowData {
        QString idCapture;
        int idBateau = 0;
        QString typePoisson;
        int quantite = 0;
        double poids = 0.0;
        QDate dateCapture;
    };

    struct SpeciesStatData {
        QString species;
        int quantite = 0;
        double pourcentage = 0.0;
    };

    bool ajouter() const;
    bool modifier() const;
    bool modifierAvecAncienId(const QString& ancienId) const;
    static bool supprimer(const QString& idCapture);
    static bool idExiste(const QString& idCapture);
    static QString genererNouvelId();
    static bool chargerTable(const QString& rechercheId,
                             const QDate& dateDebut,
                             QVector<TableRowData>& rows);
    static bool chargerTableAvancee(const QString& rechercheId,
                                    int quantiteFiltre,
                                    const QDate& dateFiltre,
                                    QVector<TableRowData>& rows);
    static QVector<SpeciesStatData> calculerTop5Species();
    static QSqlQueryModel* afficher();
    static QString
    lastError();

private:
    QString idCapture_;
    int idBateau_ = 0;
    QString typePoisson_;
    int quantite_ = 0;
    double poids_ = 0.0;
    QDate dateCapture_;
};

#endif // CAPTURES_H
