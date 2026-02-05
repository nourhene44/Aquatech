#ifndef GPECHEURS_H
#define GPECHEURS_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class Gpecheurs;
}
QT_END_NAMESPACE

class Gpecheurs : public QMainWindow
{
    Q_OBJECT

public:
    Gpecheurs(QWidget *parent = nullptr);
    ~Gpecheurs();

private slots:
    void on_btnFaceIDp_clicked();  // Slot pour le bouton FaceID
    void on_pushButton_11p_clicked();
    void on_bmi_6p_clicked();  // Bouton Valider dans la frame faceid
    // Slot pour le bouton de retour/annuler

private:
    Ui::Gpecheurs *ui;
};

#endif // GPECHEURS_H
