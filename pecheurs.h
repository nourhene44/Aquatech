#ifndef PECHEURS_H
#define PECHEURS_H

#include <QString>
#include <QDate>
#include <QVector>
#include <QSqlQueryModel>

class Pecheurs
{
public:
    struct TableRowData {
        QString id;
        QString nom;
        QString prenom;
        QString sexe;
        QString role;
        int idBateau = 0;
        QString disponibilite;
        QString email;
        QDate dateInscription;
        QDate dateAffectation;
        int heures = 0;
        QString photo;
    };

    struct DisponibiliteStats {
        int disponible = 0;
        int bientot = 0;
        int indisponible = 0;
        int enConge = 0;
    };

    Pecheurs();
    Pecheurs(const QString& id, const QString& nom, const QString& prenom,
             const QString& sexe,
             const QString& role, const QString& disponibilite,
             const QString& email, int heures,
             const QDate& dateInscription, const QDate& dateAffectation,
             int idBateau);

    bool ajouter() const;
    static bool supprimer(const QString& id);
    bool modifier() const;
    bool modifierAvecAncienId(const QString& ancienId) const;
    static QSqlQueryModel* afficher();
    static bool chargerTable(const QString& recherche,
                             const QString& roleSelection,
                             const QString& dispoSelection,
                             QVector<TableRowData>& rows);
    static DisponibiliteStats calculerDisponibiliteStats(const QString& recherche,
                                                         const QString& roleSelection,
                                                         const QString& dispoSelection);
    static bool idExiste(const QString& id);
    static QString genererNouvelId(const QString& sexe);
    static int bateauIdFromText(const QString& text);
    static QString lastError();

private:
    QString id_;
    QString nom_;
    QString prenom_;
    QString sexe_;
    QString role_;
    QString disponibilite_;
    QString email_;
    int heures_;
    QDate dateInscription_;
    QDate dateAffectation_;
    int idBateau_;
};

#endif // PECHEURS_H
