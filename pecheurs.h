#ifndef PECHEURS_H
#define PECHEURS_H

#include <QString>
#include <QDate>
#include <QSqlQueryModel>

class Pecheurs
{
public:
    Pecheurs();
    Pecheurs(int id, const QString& nom, const QString& prenom,
             const QString& role, const QString& disponibilite,
             const QString& email, int heures,
             const QDate& dateInscription, const QDate& dateAffectation,
             int idBateau);

    bool ajouter() const;
    static bool supprimer(int id);
    bool modifier() const;
    static QSqlQueryModel* afficher();
    static QString lastError();
    
    // Setters
    void setId(int id) { id_ = id; }

private:
    int id_;
    QString nom_;
    QString prenom_;
    QString role_;
    QString disponibilite_;
    QString email_;
    int heures_;
    QDate dateInscription_;
    QDate dateAffectation_;
    int idBateau_;
};

#endif // PECHEURS_H
