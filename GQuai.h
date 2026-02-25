#ifndef GQUAI_H
#define GQUAI_H

#include <QMainWindow>
#include <QString>
#include <QMessageBox>
#include <QProgressBar>
#include <QLabel>

#include "connection.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// (Removed direct OCCI dependency; using the project's `Connection` wrapper)

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Database connection methods - PUBLIC so they can be accessed
    Connection* getOracleConnection();
    void closeOracleConnection(Connection* conn);

    // CRUD Operations
    void ajouterBateau(const QString& nom, const QString& type, int capacite,
                       const QString& proprietaire, const QString& statut,
                       double largeur);
    void ajouterBateauFromForm(); // Will be called by the Ajouter button

private slots:
    // Navigation GBateau (login / récupération)
    void on_pushButton_3b_17_clicked();void on_pushButton_3b_54_clicked();void on_pushButton_3b_48_clicked();void on_pushButton_3b_5_clicked();
    void on_pushButton_3b_46_clicked();void on_pushButton_3b_11_clicked();void on_pushButton_3b_42_clicked();
    void on_pushButton_3b_4_clicked();void on_pushButton_3b_10_clicked();void on_pushButton_3b_16_clicked();void on_pushButton_3b_53_clicked();
     void on_pushButton_3b_41_clicked(); void on_pushButton_3b_40_clicked();void on_pushButton_3b_47_clicked();
      void on_pushButton_3b_52_clicked(); void on_pushButton_3b_55_clicked();
     void on_pushButton_3b_15_clicked(); void on_pushButton_3b_43_clicked();void on_pushButton_3b_49_clicked();void on_pushButton_3b_6_clicked();
     void on_pushButton_3b_3_clicked();     void on_pushButton_3b_18_clicked();     void on_pushButton_3b_12_clicked();
     void on_pushButton_3b_9_clicked();  void on_pushButton_3b_19_clicked(); void on_pushButton_3b_56_clicked(); void on_pushButton_3b_44_clicked();
    void on_pushButton_3b_45_clicked(); void on_pushButton_3b_50_clicked(); void on_pushButton_3b_7_clicked();void on_pushButton_3b_13_clicked();
    void on_pushButton_3b_39_clicked();
    void on_pushButton_3b_51_clicked();
    void on_pushButton_3b_8_clicked();
    void on_pushButton_3b_2_clicked();
    void on_pushButton_3b_14_clicked();
    void on_pushButton_b_2b_clicked();
    void on_pushButton_b_3b_clicked();
    void on_pushButton_b_clicked();
    void on_pushButton_2b_clicked();
    void on_pushButton_b_4b_clicked();    // Connexion -> menu (gestionsb)
    void on_pushButton_20b_clicked();  // Mot de passe oublié? -> Rmdpb
    void on_pushButton_13b_clicked();  // Envoyer -> Remailb
    void on_pushButton_14b_clicked();  // Annuler -> loginb
    void on_pushButton_15b_clicked();  // Verifier -> Nmdpb
    void on_pushButton_16b_clicked();  // Valider (nouveau mdp) -> loginb
    void on_pushButton_17b_clicked();  // Retour -> loginb
    void on_pushButton_18b_clicked();  // Retour -> loginb
    void on_pushButton_19b_clicked();  // Retour -> loginb

    // Navigation GBateau (menu)
    void on_p2b_clicked();            // Menu GBateau -> pagepecheur
    void on_p1b_clicked();            // Menu GBateau -> gestionbateaub
    void on_p3b_clicked();            // Menu GBateau -> Gestion des clients (fenêtre dédiée)
    void on_pushButton_3b_clicked();  // Retour (gestionbateaub) -> menu GBateau
    void on_p1_2b_clicked();          // Toggle frame_10b (GBateau)
    void on_pushButton_12b_clicked(); // Fermer (frame_10b)
    void on_pushButton_9b_clicked();  // Déconnexion -> loginb

    // Page pêcheurs : FaceID
    void on_btnFaceIDp_clicked();      // Afficher framefaceidp
    void on_bmi_6p_clicked();          // Valider FaceID (fermer)
    void on_pushButton_11p_clicked();  // Annuler FaceID (fermer)
    void on_brmp_clicked();            // Page pêcheurs -> menu (gestionsb)

    // Gestion des quais
    void on_p4b_clicked();          // Menu GBateau -> Gestion des quais (page_3)
    void on_pushButton_clicked();   // Retour Menu (page_3) -> Menu GBateau

    // Gestion des employés
    void on_p6b_clicked();          // Menu GBateau -> Gestion des employés (pagee)
    void on_pushButton_10e_clicked();
    void on_pushButton_pdfb_5_clicked();    // Retour Menu (pagee) -> Menu GBateau
    void on_pushButton_7c_5_clicked();
    void on_pushButton_8c_4_clicked();
    void on_pushButton_8c_3_clicked();
    // Gestion des captures
    void on_p5b_clicked();          // Menu GBateau -> Gestion des captures
    void on_cap_btnBackMenu_clicked();
    void on_cap_btnShowSurpeche_clicked();
    void on_cap_btnShowTendance_clicked();
    void on_cap_btnBackToCapturesMainSurpeche_clicked();
    void on_cap_btnBackToCapturesMainTendance_clicked();

    // PDF export
    void on_pushButton_pdfb_clicked(); // Exporter PDF bateaux

    // Alertes
    void on_pushButton_7b_clicked(); // Afficher le panneau des alertes (framealertb)

    // Page clients (intégrée dans mainwindow.ui)
    void on_btnai_clicked();          // Toggle panneau chat IA
    void on_btnSend_2_clicked();      // Fermer panneau chat IA
    void on_pushButton_7c_clicked();  // Retour Menu depuis clients

    // Map et statistiques
    void on_pushButton_6_clicked();  // Voir Map
    void on_pushButton_2_clicked();  // Retour
    void on_pushButton_9_clicked();  // Voir les statistiques

    // Actions table bateaux
    void modifierBateauFromRow(int row);
    void supprimerBateauFromRow(int row);
    void filterBateaux();

private:
    Ui::MainWindow *ui;

    // ID of the bateau currently being edited (empty when adding)
    QString currentEditingId;

    // Reload the bateaux table from the database
    void loadBateaux();

    // Update statistics widgets for bateaux (counts by statut/type)
    void updateStatsBateaux();

    void showFrame(QWidget* frameToShow);
    void setupFrames();
    void normalizeUiTexts();

    // (Database load/validation helpers removed)
};

#endif // GQUAI_H
