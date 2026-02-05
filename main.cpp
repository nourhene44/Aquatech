#include "gpecheurs.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Gpecheurs w;
    w.show();
    return a.exec();
}
