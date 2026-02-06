#ifndef GCLIENTS_H
#define GCLIENTS_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void on_btnSend_2_clicked();
   void on_btnai_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // GCLIENTS_H
