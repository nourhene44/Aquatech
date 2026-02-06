#include "Gclients.h"
#include "ui_Gclients.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->frame_2c_2->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::on_btnSend_2_clicked()
{
    ui->frame_2c_2->setVisible(!ui->frame_2c_2->isVisible());
}
void MainWindow::on_btnai_clicked()
{
    ui->frame_2c_2->setVisible(!ui->frame_2c_2->isVisible());
}
