#include "GQuai.h"
#include "ui_GQuai.h"
#include <QPushButton>
#include <QMessageBox>
#include <QMouseEvent>
#include <QTimer>
#include <QRandomGenerator>  // 添加随机数生成器头文件
#include <QTime>            // 确保 QTime 已包含
#include <QtGlobal>

static QString formatZoneSummary(const QString& zoneName, int occupied, int total, int maintenance)
{
    const int free = qMax(0, total - occupied - maintenance);
    return QString("%1 : %2/%3 occupés • %4 libres • %5 en maintenance")
        .arg(zoneName)
        .arg(occupied)
        .arg(total)
        .arg(free)
        .arg(maintenance);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Initialiser les données de la carte
    initMapData();

    // Configurer les connexions pour la carte
    setupMapConnections();

    // Mettre à jour l'affichage initial
    refreshMap();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initMapData()
{
    zoneQuaiCount["Nord"] = 20;
    zoneQuaiCount["Sud"] = 16;
    zoneQuaiCount["Est"] = 12;
    zoneQuaiCount["Ouest"] = 14;

    zoneQuaiOccupied["Nord"] = 8;
    zoneQuaiOccupied["Sud"] = 6;
    zoneQuaiOccupied["Est"] = 4;
    zoneQuaiOccupied["Ouest"] = 5;

    zoneQuaiMaintenance["Nord"] = 1;
    zoneQuaiMaintenance["Sud"] = 0;
    zoneQuaiMaintenance["Est"] = 1;
    zoneQuaiMaintenance["Ouest"] = 0;
}

void MainWindow::setupMapConnections()
{
    if (ui->btnRefreshMap) {
        connect(ui->btnRefreshMap, &QPushButton::clicked, this, &MainWindow::onRefreshMapClicked);
    }
}

void MainWindow::updateZoneColor(QFrame* zone, int occupied, int total, int maintenance)
{
    if (!zone || total <= 0) {
        return;
    }

    const int used = occupied + maintenance;
    const double ratio = static_cast<double>(qMin(used, total)) / static_cast<double>(total);

    QString color;
    if (maintenance > 0) {
        color = "#FF8C00"; // orange
    } else if (ratio >= 0.80) {
        color = "#E74C3C"; // rouge
    } else if (ratio >= 0.50) {
        color = "#F1C40F"; // jaune
    } else {
        color = "#2ECC71"; // vert
    }

    zone->setStyleSheet(QString("QFrame#%1 { background-color: %2; border: 2px solid #59abc8; border-radius: 15px; }")
                            .arg(zone->objectName(), color));
}

void MainWindow::updateZoneDisplay(const QString& zoneName, QLabel* label)
{
    if (!label) {
        return;
    }
    const int total = zoneQuaiCount.value(zoneName, 0);
    const int occupied = zoneQuaiOccupied.value(zoneName, 0);
    const int maintenance = zoneQuaiMaintenance.value(zoneName, 0);
    label->setText(formatZoneSummary(zoneName, occupied, total, maintenance));
}

void MainWindow::refreshMap()
{
    updateZoneDisplay("Nord", ui->value_zoneNord_map);
    updateZoneDisplay("Sud", ui->value_zoneSud_map);
    updateZoneDisplay("Est", ui->value_zoneEst_map);
    updateZoneDisplay("Ouest", ui->value_zoneOuest_map);

    updateZoneColor(ui->zoneNord_map, zoneQuaiOccupied.value("Nord"), zoneQuaiCount.value("Nord"), zoneQuaiMaintenance.value("Nord"));
    updateZoneColor(ui->zoneSud_map, zoneQuaiOccupied.value("Sud"), zoneQuaiCount.value("Sud"), zoneQuaiMaintenance.value("Sud"));
    updateZoneColor(ui->zoneEst_map, zoneQuaiOccupied.value("Est"), zoneQuaiCount.value("Est"), zoneQuaiMaintenance.value("Est"));
    updateZoneColor(ui->zoneOuest_map, zoneQuaiOccupied.value("Ouest"), zoneQuaiCount.value("Ouest"), zoneQuaiMaintenance.value("Ouest"));

    if (ui->label_map_info && ui->label_map_info->text().isEmpty()) {
        ui->label_map_info->setText("Cliquez sur une zone pour voir les détails");
    }
}

void MainWindow::updateMapDisplay()
{
    refreshMap();
}

void MainWindow::onZoneNordClicked()
{
    if (ui->label_map_info) {
        ui->label_map_info->setText(formatZoneSummary("Nord", zoneQuaiOccupied.value("Nord"), zoneQuaiCount.value("Nord"), zoneQuaiMaintenance.value("Nord")));
    }
}

void MainWindow::onZoneSudClicked()
{
    if (ui->label_map_info) {
        ui->label_map_info->setText(formatZoneSummary("Sud", zoneQuaiOccupied.value("Sud"), zoneQuaiCount.value("Sud"), zoneQuaiMaintenance.value("Sud")));
    }
}

void MainWindow::onZoneEstClicked()
{
    if (ui->label_map_info) {
        ui->label_map_info->setText(formatZoneSummary("Est", zoneQuaiOccupied.value("Est"), zoneQuaiCount.value("Est"), zoneQuaiMaintenance.value("Est")));
    }
}

void MainWindow::onZoneOuestClicked()
{
    if (ui->label_map_info) {
        ui->label_map_info->setText(formatZoneSummary("Ouest", zoneQuaiOccupied.value("Ouest"), zoneQuaiCount.value("Ouest"), zoneQuaiMaintenance.value("Ouest")));
    }
}

void MainWindow::on_pushButton_6_clicked()
{
    if (ui->stackedWidget) {
        ui->stackedWidget->setCurrentWidget(ui->page_4);
    }
    refreshMap();
}

void MainWindow::on_pushButton_2_clicked()
{
    if (ui->stackedWidget) {
        ui->stackedWidget->setCurrentWidget(ui->page_3);
    }
}

void MainWindow::on_pushButton_9_clicked()
{
    if (ui->stackedWidget) {
        ui->stackedWidget->setCurrentWidget(ui->page_4);
    }
    refreshMap();
}

void MainWindow::onRefreshMapClicked()
{
    // 使用 QRandomGenerator 替代废弃的 qrand()
    QRandomGenerator *generator = QRandomGenerator::global();

    // Simuler la mise à jour des données
    zoneQuaiOccupied["Nord"] = generator->bounded(zoneQuaiCount["Nord"] + 1);
    zoneQuaiOccupied["Sud"] = generator->bounded(zoneQuaiCount["Sud"] + 1);
    zoneQuaiOccupied["Est"] = generator->bounded(zoneQuaiCount["Est"] + 1);
    zoneQuaiOccupied["Ouest"] = generator->bounded(zoneQuaiCount["Ouest"] + 1);

    zoneQuaiMaintenance["Nord"] = generator->bounded(3);
    zoneQuaiMaintenance["Sud"] = generator->bounded(2);
    zoneQuaiMaintenance["Est"] = generator->bounded(2);
    zoneQuaiMaintenance["Ouest"] = generator->bounded(3);

    refreshMap();

    // Afficher un message temporaire
    ui->label_map_info->setText("✅ Carte mise à jour à " + QTime::currentTime().toString("hh:mm:ss"));
    QTimer::singleShot(3000, this, [this]() {
        ui->label_map_info->setText("Cliquez sur une zone pour voir les détails");
    });
}

