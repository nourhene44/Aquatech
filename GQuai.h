#ifndef GQUAI_H
#define GQUAI_H

#include <QMainWindow>
#include <QMap>
#include <QString>
#include <QLabel>      // 添加 QLabel 头文件
#include <QFrame>      // 添加 QFrame 头文件
#include <QTime>       // 添加 QTime 头文件

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
    void on_pushButton_6_clicked();  // Voir Map
    void on_pushButton_2_clicked();  // Retour
    void on_pushButton_9_clicked();  // Voir les statistiques

    // Fonctions pour la carte
    void updateMapDisplay();
    void refreshMap();
    void onZoneNordClicked();
    void onZoneSudClicked();
    void onZoneEstClicked();
    void onZoneOuestClicked();
    void onRefreshMapClicked();

private:
    Ui::MainWindow *ui;

    // Données pour la carte
    QMap<QString, int> zoneQuaiCount;
    QMap<QString, int> zoneQuaiOccupied;
    QMap<QString, int> zoneQuaiMaintenance;

    void initMapData();
    void setupMapConnections();
    void updateZoneDisplay(const QString& zoneName, QLabel* label);
    void updateZoneColor(QFrame* zone, int occupied, int total, int maintenance);
};
#endif // GQUAI_H
