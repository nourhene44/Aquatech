#include "gpecheurs.h"
#include "ui_gpecheurs.h"

Gpecheurs::Gpecheurs(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Gpecheurs)
{
    ui->setupUi(this);

    // Initialiser l'état des widgets au démarrage
    ui->framefaceidp->setVisible(false);
    ui->tableWidgetp->setVisible(true);   // Afficher le tableau
    ui->frame_typesp->setVisible(true);    // Cacher la frame faceid au départ
}

Gpecheurs::~Gpecheurs()
{
    delete ui;
}

// Slot appelé quand on clique sur le bouton FaceID
void Gpecheurs::on_btnFaceIDp_clicked()
{
    // Afficher la frame d'enregistrement FaceID
    ui->framefaceidp->setVisible(true);

    // Cacher les autres éléments (tableau et statistiques)
    ui->tableWidgetp->setVisible(false);
    ui->frame_typesp->setVisible(false);
    ui->lineEdit_4p->setVisible(false);
    ui->label_3p->setVisible(false);
    ui->label_2p->setVisible(false);
    ui->comboBox_5p->setVisible(false);
    ui->label_4p->setVisible(false);
    ui->comboBox_6p->setVisible(false);
    ui->pushButton_4p->setVisible(false);
    ui->pushButton_5p->setVisible(false);
    ui->pushButton_6p->setVisible(false);

    // Si vous avez d'autres éléments à cacher, ajoutez-les ici :
    // ui->autre_widget->setVisible(false);
}
void Gpecheurs::on_bmi_6p_clicked()
{
    // 1. Ici, ajoutez votre logique de validation du visage
    // Par exemple : sauvegarder les données, traiter l'image, etc.

    // 2. Cacher la frame faceid
    ui->framefaceidp->setVisible(false);

    // 3. Réafficher les autres éléments
    ui->tableWidgetp->setVisible(true);
    ui->frame_typesp->setVisible(true);
    ui->lineEdit_4p->setVisible(true);
    ui->label_3p->setVisible(true);
    ui->label_2p->setVisible(true);
    ui->comboBox_5p->setVisible(true);
    ui->label_4p->setVisible(true);
    ui->comboBox_6p->setVisible(true);
    ui->pushButton_4p->setVisible(true);
    ui->pushButton_5p->setVisible(true);
    ui->pushButton_6p->setVisible(true);

    // Optionnel : actualiser les données dans le tableau
    // actualiserTableau();
}

// Slot appelé quand on clique sur le bouton de retour/annuler (pushButton_11)
void Gpecheurs::on_pushButton_11p_clicked()
{
    // Cacher la frame faceid
    ui->framefaceidp->setVisible(false);
    ui->tableWidgetp->setVisible(true);
    ui->frame_typesp->setVisible(true);
    ui->lineEdit_4p->setVisible(true);
    ui->label_3p->setVisible(true);
    ui->label_2p->setVisible(true);
    ui->comboBox_5p->setVisible(true);
    ui->label_4p->setVisible(true);
    ui->comboBox_6p->setVisible(true);
    ui->pushButton_4p->setVisible(true);
    ui->pushButton_5p->setVisible(true);
    ui->pushButton_6p->setVisible(true);

    // Réafficher les autres éléments si nécessaire
    // ui->tableWidget->setVisible(true);
    // ui->frame_types->setVisible(true);
}
