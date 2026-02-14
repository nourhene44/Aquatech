#include "GQuai.h"
#include <QApplication>
#include "connection.h"
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // ✅ Utilisation du Singleton - PAS de constructeur direct!
    Connection* conn = Connection::getInstance();

    if(!conn->createconnect())
    {
        QMessageBox::critical(nullptr, "Database Error",
                              "❌ Connection to Oracle failed.\nPlease check your database settings.");
        return -1;
    }
    else
    {
        QMessageBox::information(nullptr, "Success",
                                 "✅ Connected to Oracle successfully!");
    }

    MainWindow w;
    w.show();

    return a.exec();
}
