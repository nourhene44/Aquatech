#ifndef EMPLOYE_H
#define EMPLOYE_H

#include <QString>
#include <QSqlQueryModel>

// Classe métier Employe suivant l'architecture de l'atelier CRUD Qt

class Employe
{
public:
    Employe();
    Employe(int id,
            const QString &nom,
            const QString &prenom,
            const QString &role,
            const QString &equipe,
            const QString &etat,
            const QString &statut,
            double salaire,
            const QString &telephoneUi = QString(),
            const QString &raison = QString(),
            const QString &cvPath = QString());

    // CRUD de base
    bool ajouter() const;
    bool modifier() const;
    bool modifierAvecAncienId(int ancienId) const;
    static bool supprimer(int id);

    // Affichage (modèle pour QTableView / remplissage de QTableWidget)
    static QSqlQueryModel *afficher();
    static int genererNouvelId(const QString &role);

    // Dernière erreur rencontrée
    static QString lastError();

    // Getters / setters simples (style atelier)
    int id() const { return m_id; }
    void setId(int id) { m_id = id; }

    QString nom() const { return m_nom; }
    void setNom(const QString &nom) { m_nom = nom; }

    QString prenom() const { return m_prenom; }
    void setPrenom(const QString &prenom) { m_prenom = prenom; }

    QString role() const { return m_role; }
    void setRole(const QString &role) { m_role = role; }

    double salaire() const { return m_salaire; }
    void setSalaire(double salaire) { m_salaire = salaire; }

    QString equipe() const { return m_equipe; }
    void setEquipe(const QString &equipe) { m_equipe = equipe; }

    QString etat() const { return m_etat; }
    void setEtat(const QString &etat) { m_etat = etat; }

    QString statut() const { return m_statut; }
    void setStatut(const QString &statut) { m_statut = statut; }

    QString raison() const { return m_raison; }
    void setRaison(const QString &raison) { m_raison = raison; }

    QString cvPath() const { return m_cvPath; }
    void setCvPath(const QString &cvPath) { m_cvPath = cvPath; }

private:
    int m_id = 0;
    QString m_nom;
    QString m_prenom;
    QString m_role;
    QString m_equipe;
    QString m_etat;
    QString m_statut;
    double m_salaire = 0.0;
    QString m_telephoneUi;
    QString m_raison;
    QString m_cvPath;

    static QString s_lastError;
};

#endif // EMPLOYE_H
