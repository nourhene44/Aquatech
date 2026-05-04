#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include "connection.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Connection* conn = Connection::getInstance();
    if (!conn->createconnect(false)) {
        qWarning() << "Oracle connection unavailable at startup:" << conn->lastErrorText();
    }

    MainWindow w;
    w.show();

    return a.exec();
}
