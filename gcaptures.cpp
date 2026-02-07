#include "gcaptures.h"
#include "ui_gcaptures.h"

Gcaptures::Gcaptures(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Gcaptures)
{
    ui->setupUi(this);
}

Gcaptures::~Gcaptures()
{
    delete ui;
}
