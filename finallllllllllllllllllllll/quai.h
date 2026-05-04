#ifndef QUAI_H
#define QUAI_H

#include <QString>
#include <QVariant>
#include <QList>

class Quai
{
public:
    Quai();

    // Constructeur pratique correspondant à la ligne de la table QUAI
    Quai(int idQuai,
         const QString &nomQuai,
         const QString &zonePort,
         const QString &zoneCouverte,
         double longueur,
         int capaciteQuais,
         const QString &statutQuai);

    // Getters / setters "comme l'atelier" pour les champs du quai
    int idQuai() const;
    void setIdQuai(int id);

    QString nomQuai() const;
    void setNomQuai(const QString &nom);

    QString zonePort() const;
    void setZonePort(const QString &zone);

    QString zoneCouverte() const;
    void setZoneCouverte(const QString &zone);

    double longueur() const;
    void setLongueur(double valeur);

    int capaciteQuais() const;
    void setCapaciteQuais(int capacite);

    QString statut() const;
    void setStatut(const QString &statut);

    // Dernière erreur SQL rencontrée (texte brut)
    QString lastError() const { return m_lastError; }

    // Génération d'ID (schéma demandé: 263NNNN). Retourne 0 si échec.
    int genererNouvelId();

    // CRUD basés sur les attributs de l'objet (style atelier)
    bool ajouter();
    bool modifier();
    bool supprimer();

    // CRUD génériques déjà utilisés dans MainWindow (compatibilité conservée)
    bool ajouter(const QVariantMap &donnees);
    QList<QVariantMap> lister();
    bool modifier(int id, const QVariantMap &donnees);
    bool supprimer(int id);

    // Méthodes pour le formulaire (pré‑remplissage, mode modification)
    QVariantMap chargerInfos(int id);
    void setModeModification(bool mode, int id = -1);
    bool isModeModification() const;
    int idEnCours() const;

private:
    // Données métier du quai
    int m_idQuai = 0;
    QString m_nomQuai;
    QString m_zonePort;
    QString m_zoneCouverte;
    double m_longueur = 0.0;
    int m_capaciteQuais = 0;
    QString m_statut;

    // Stockage du dernier message d'erreur SQL
    QString m_lastError;

    // État de modification (utilisé par l'IHM)
    bool m_modeModification = false;
    int m_idEnCours = -1;
    QVariantMap m_donneesEnCours;
};

#endif // QUAI_H