#ifndef GCAPTURES_H
#define GCAPTURES_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class Gcaptures; }
QT_END_NAMESPACE

class Gcaptures : public QMainWindow
{
    Q_OBJECT

public:
    explicit Gcaptures(QWidget *parent = nullptr);
    ~Gcaptures();

private:
    Ui::Gcaptures *ui;
};

#endif // GCAPTURES_H
