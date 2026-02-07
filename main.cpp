#include "gcaptures.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Gcaptures w;
    w.show();
    return a.exec();
}
