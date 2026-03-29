#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QCoreApplication>
#include "connection.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Add library paths IMMEDIATELY after QApplication creation, BEFORE any DB operations
    const QString appDir = QCoreApplication::applicationDirPath();
    QCoreApplication::addLibraryPath(appDir);
    QCoreApplication::addLibraryPath(appDir + QStringLiteral("/plugins"));
    qDebug() << "[DB] Library paths set to:" << QCoreApplication::libraryPaths();

    Connection* c = Connection::getInstance();
    bool test = c->createconnect();
    qDebug() << "[DB] Drivers available:" << c->availableDrivers();
    qDebug() << "[DB] Selected driver:" << c->selectedDriver();

    if (test)
    {
        MainWindow w;
        w.show();
        QMessageBox::information(nullptr, QObject::tr("database is open"),
                                 QObject::tr("connection successful.\nDriver: %1\nAvailable: %2\n"
                                             "Click Cancel to exit.")
                                     .arg(c->selectedDriver(), c->availableDrivers().join(", ")),
                                 QMessageBox::Cancel);

        return a.exec();
    }

    QMessageBox::critical(nullptr, QObject::tr("database is not open"),
                          QObject::tr("connection failed.\nAvailable drivers: %1\nLast error: %2\n"
                                      "Click Cancel to exit.")
                              .arg(c->availableDrivers().join(", "), c->lastError()),
                          QMessageBox::Cancel);

    return 0;
}
