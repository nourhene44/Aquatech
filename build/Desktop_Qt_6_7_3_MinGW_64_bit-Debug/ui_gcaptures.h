/********************************************************************************
** Form generated from reading UI file 'gcaptures.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GCAPTURES_H
#define UI_GCAPTURES_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Gcaptures
{
public:
    QWidget *centralwidget;
    QStackedWidget *stackedWidget;
    QWidget *page;
    QFrame *frameStatsZones;
    QVBoxLayout *lytStatsZones;
    QLabel *label_zones;
    QHBoxLayout *zonesContainerLayout;
    QVBoxLayout *zoneNordLayout;
    QLabel *label_zoneNord;
    QProgressBar *progressZoneNord;
    QLabel *value_zoneNord;
    QVBoxLayout *zoneSudLayout;
    QLabel *label_zoneSud;
    QProgressBar *progressZoneSud;
    QLabel *value_zoneSud;
    QVBoxLayout *zoneEstLayout;
    QLabel *label_zoneEst;
    QProgressBar *progressZoneEst;
    QLabel *value_zoneEst;
    QVBoxLayout *zoneOuestLayout;
    QLabel *label_zoneOuest;
    QProgressBar *progressZoneOuest;
    QLabel *value_zoneOuest;
    QLabel *label_10;
    QLineEdit *leSearch;
    QFrame *frameLeft;
    QSpinBox *sbQuantite_2;
    QDoubleSpinBox *dsPoids_2;
    QDateEdit *deDateCapture_2;
    QLineEdit *lineEdit_5;
    QLineEdit *lineEdit_6;
    QLabel *label_5;
    QLabel *label_6;
    QLabel *label_7;
    QPushButton *btnAffecter_2;
    QPushButton *btnModifier_2;
    QPushButton *btnValiider_2;
    QLabel *label_8;
    QComboBox *comboBox_2;
    QPushButton *btnReafraichir;
    QPushButton *btnEditLefft;
    QTableWidget *tableWidget_2;
    QPushButton *btnExporter_2;
    QDateEdit *deDebut;
    QPushButton *btnExporter;
    QLabel *lblTitle;
    QFrame *frame;
    QLineEdit *lineEdit_7;
    QLineEdit *lineEdit_8;
    QPushButton *btnExporter_5;
    QPushButton *btnExporter_6;
    QPushButton *pushButton;
    QPushButton *btnAjouter;
    QSpinBox *sbQuantite_3;
    QLabel *labelLeftTitle;
    QLabel *label_11;
    QTableWidget *tableWidget;
    QWidget *page_3;
    QLabel *lblTitle_3;
    QFrame *frame_3;
    QComboBox *comboBox_3;
    QLabel *label_27;
    QLabel *label_28;
    QDateEdit *dateEdit;
    QLabel *label_29;
    QDateEdit *dateEdit_2;
    QPushButton *pushButton_4;
    QLabel *label_30;
    QGroupBox *groupStatsBateau;
    QLabel *label_idBateau;
    QLabel *lblIdBateauValue;
    QLabel *label_31;
    QLabel *label_32;
    QLabel *label_33;
    QLabel *label_34;
    QPushButton *pushButton_5;
    QWidget *page_2;
    QLabel *lblTitle_2;
    QFrame *frame_2;
    QLabel *label_19;
    QLabel *label_20;
    QLabel *label_21;
    QTableWidget *tableWidget_4;
    QLabel *label_22;
    QGroupBox *groupBox_3;
    QLabel *label_23;
    QTextBrowser *textBrowser_2;
    QGroupBox *groupBox_4;
    QProgressBar *progressBar_4;
    QProgressBar *progressBar_5;
    QProgressBar *progressBar_6;
    QLabel *label_24;
    QLabel *label_25;
    QLabel *label_26;
    QPushButton *pushButton_3;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *Gcaptures)
    {
        if (Gcaptures->objectName().isEmpty())
            Gcaptures->setObjectName("Gcaptures");
        Gcaptures->resize(1476, 880);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(Gcaptures->sizePolicy().hasHeightForWidth());
        Gcaptures->setSizePolicy(sizePolicy);
        Gcaptures->setStyleSheet(QString::fromUtf8("QComboBox,QSpinBox,QDoubleSpinBox,QDateEdit {\n"
"    font-size: 16px;\n"
"    padding: 6px;\n"
"    border: 2px solid #59abc8;\n"
"    border-radius: 15px;\n"
"    background-color: rgba(0, 0, 50, 0.8);\n"
"    color:white;\n"
"}\n"
"\n"
"QTableWidget {\n"
"    background-color: rgba(0, 0, 50, 0.7);\n"
"    border: 2px solid #59abc8;\n"
"    border-radius: 15px;\n"
"    gridline-color: #59abc8;\n"
"    color: white;\n"
"}\n"
"\n"
"QTableWidget::item {\n"
"    padding: 5px;\n"
"    border-bottom: 1px solid rgba(89, 171, 200, 0.3);\n"
"}\n"
"\n"
"QTableWidget::item:selected {\n"
"    background-color: #0078D7;\n"
"}\n"
"\n"
"QHeaderView::section {\n"
"    background-color: rgba(0, 40, 80, 0.9);\n"
"    color: white;\n"
"    padding: 8px;\n"
"    border-radius: 5px;\n"
"    border: 1px solid #59abc8;\n"
"}\n"
"\n"
"/* IMPORTANT : Avec QMainWindow, on stylise centralwidget */\n"
"#centralwidget {\n"
"    background-image: url(:/image/bg.png);\n"
"    background-position: center;\n"
"    background-size: cover;\n"
""
                        "    border-radius:15px;\n"
"}\n"
"\n"
"#lblTitle{\n"
"    border:none;\n"
"    background-color: transparent;\n"
"    font-weight: 700;\n"
"    color: rgb(66, 157, 255);\n"
"    font: 25pt \"Imprint MT Shadow\";\n"
"}\n"
"\n"
"#label,#label_2,#label_3,#label_4,#label_10,#label_11{\n"
"    border:none;\n"
"    background-color:transparent;\n"
"    font-size: 15px;\n"
"    padding: 1px;\n"
"}\n"
"\n"
"#labelLeftTitle{\n"
"    color:#fffae0;\n"
"    font-weight: 600;\n"
"    font-size: 17px;\n"
"    padding: 5px 0;\n"
"}\n"
"\n"
"#label_2b,#label_4b,#label_8b,#label_13b{\n"
"    background-color:transparent;\n"
"    border-image: url(:/image/login.png);\n"
"}\n"
"\n"
"QLabel{\n"
"    font-family: 'Arial';\n"
"    color:white;\n"
"}\n"
"\n"
"QFrame {\n"
"    border-radius: 15px;\n"
"    background-color: rgba(0, 0, 50, 0.7);\n"
"    border: 2px solid #59abc8 ;\n"
"}\n"
"\n"
"QPushButton {\n"
"    border-radius: 20px;\n"
"    background-color: rgba(0, 0, 150, 0.7);\n"
"    border: 2px solid #59abc8;\n"
"    color: "
                        "white;\n"
"    font-weight: 700;\n"
"    font-size: 15px;\n"
"    margin-bottom:20px;\n"
"    padding: 10px 20px;\n"
"}\n"
"\n"
"QPushButton:hover {\n"
"    color: #59abc8;\n"
"}\n"
"\n"
"QLineEdit{\n"
"    font-size: 17px;\n"
"    background: transparent;\n"
"    border: none;\n"
"    color: white;\n"
"    border-bottom: 2px solid #59abc8;\n"
"    padding: 3px;\n"
"    selection-background-color: white;\n"
"}\n"
"\n"
"QLineEdit:focus {\n"
"    color: #59abc8;\n"
"}\n"
"\n"
"*{\n"
"  font-family: \"Segoe UI\", \"Century Gothic\", sans-serif;\n"
"  font-size: 14px;\n"
"  font-weight: 400;\n"
"}\n"
""));
        centralwidget = new QWidget(Gcaptures);
        centralwidget->setObjectName("centralwidget");
        stackedWidget = new QStackedWidget(centralwidget);
        stackedWidget->setObjectName("stackedWidget");
        stackedWidget->setGeometry(QRect(0, 0, 1691, 951));
        stackedWidget->setStyleSheet(QString::fromUtf8("#page{background-image: url(:/images/bg.png);}"));
        page = new QWidget();
        page->setObjectName("page");
        frameStatsZones = new QFrame(page);
        frameStatsZones->setObjectName("frameStatsZones");
        frameStatsZones->setGeometry(QRect(690, 470, 760, 300));
        frameStatsZones->setFrameShape(QFrame::StyledPanel);
        frameStatsZones->setFrameShadow(QFrame::Raised);
        lytStatsZones = new QVBoxLayout(frameStatsZones);
        lytStatsZones->setObjectName("lytStatsZones");
        label_zones = new QLabel(frameStatsZones);
        label_zones->setObjectName("label_zones");
        label_zones->setStyleSheet(QString::fromUtf8("color: white;\n"
"    font-weight: bold;\n"
"    margin-top: 10px;"));

        lytStatsZones->addWidget(label_zones);

        zonesContainerLayout = new QHBoxLayout();
        zonesContainerLayout->setObjectName("zonesContainerLayout");
        zoneNordLayout = new QVBoxLayout();
        zoneNordLayout->setObjectName("zoneNordLayout");
        label_zoneNord = new QLabel(frameStatsZones);
        label_zoneNord->setObjectName("label_zoneNord");
        label_zoneNord->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;"));
        label_zoneNord->setAlignment(Qt::AlignCenter);

        zoneNordLayout->addWidget(label_zoneNord);

        progressZoneNord = new QProgressBar(frameStatsZones);
        progressZoneNord->setObjectName("progressZoneNord");
        progressZoneNord->setMinimumSize(QSize(30, 120));
        progressZoneNord->setOrientation(Qt::Vertical);
        progressZoneNord->setTextDirection(QProgressBar::TopToBottom);
        progressZoneNord->setValue(0);

        zoneNordLayout->addWidget(progressZoneNord);

        value_zoneNord = new QLabel(frameStatsZones);
        value_zoneNord->setObjectName("value_zoneNord");
        value_zoneNord->setStyleSheet(QString::fromUtf8("color: #3498db; font-weight: bold; font-size: 12px;"));
        value_zoneNord->setAlignment(Qt::AlignCenter);

        zoneNordLayout->addWidget(value_zoneNord);


        zonesContainerLayout->addLayout(zoneNordLayout);

        zoneSudLayout = new QVBoxLayout();
        zoneSudLayout->setObjectName("zoneSudLayout");
        label_zoneSud = new QLabel(frameStatsZones);
        label_zoneSud->setObjectName("label_zoneSud");
        label_zoneSud->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;"));
        label_zoneSud->setAlignment(Qt::AlignCenter);

        zoneSudLayout->addWidget(label_zoneSud);

        progressZoneSud = new QProgressBar(frameStatsZones);
        progressZoneSud->setObjectName("progressZoneSud");
        progressZoneSud->setMinimumSize(QSize(30, 120));
        progressZoneSud->setOrientation(Qt::Vertical);
        progressZoneSud->setTextDirection(QProgressBar::TopToBottom);
        progressZoneSud->setValue(0);

        zoneSudLayout->addWidget(progressZoneSud);

        value_zoneSud = new QLabel(frameStatsZones);
        value_zoneSud->setObjectName("value_zoneSud");
        value_zoneSud->setStyleSheet(QString::fromUtf8("color: #27ae60; font-weight: bold; font-size: 12px;"));
        value_zoneSud->setAlignment(Qt::AlignCenter);

        zoneSudLayout->addWidget(value_zoneSud);


        zonesContainerLayout->addLayout(zoneSudLayout);

        zoneEstLayout = new QVBoxLayout();
        zoneEstLayout->setObjectName("zoneEstLayout");
        label_zoneEst = new QLabel(frameStatsZones);
        label_zoneEst->setObjectName("label_zoneEst");
        label_zoneEst->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;"));
        label_zoneEst->setAlignment(Qt::AlignCenter);

        zoneEstLayout->addWidget(label_zoneEst);

        progressZoneEst = new QProgressBar(frameStatsZones);
        progressZoneEst->setObjectName("progressZoneEst");
        progressZoneEst->setMinimumSize(QSize(30, 120));
        progressZoneEst->setOrientation(Qt::Vertical);
        progressZoneEst->setTextDirection(QProgressBar::TopToBottom);
        progressZoneEst->setValue(0);

        zoneEstLayout->addWidget(progressZoneEst);

        value_zoneEst = new QLabel(frameStatsZones);
        value_zoneEst->setObjectName("value_zoneEst");
        value_zoneEst->setStyleSheet(QString::fromUtf8("color: #f39c12; font-weight: bold; font-size: 12px;"));
        value_zoneEst->setAlignment(Qt::AlignCenter);

        zoneEstLayout->addWidget(value_zoneEst);


        zonesContainerLayout->addLayout(zoneEstLayout);

        zoneOuestLayout = new QVBoxLayout();
        zoneOuestLayout->setObjectName("zoneOuestLayout");
        label_zoneOuest = new QLabel(frameStatsZones);
        label_zoneOuest->setObjectName("label_zoneOuest");
        label_zoneOuest->setStyleSheet(QString::fromUtf8("color: white; font-weight: bold;"));
        label_zoneOuest->setAlignment(Qt::AlignCenter);

        zoneOuestLayout->addWidget(label_zoneOuest);

        progressZoneOuest = new QProgressBar(frameStatsZones);
        progressZoneOuest->setObjectName("progressZoneOuest");
        progressZoneOuest->setMinimumSize(QSize(30, 120));
        progressZoneOuest->setOrientation(Qt::Vertical);
        progressZoneOuest->setTextDirection(QProgressBar::TopToBottom);
        progressZoneOuest->setValue(0);

        zoneOuestLayout->addWidget(progressZoneOuest);

        value_zoneOuest = new QLabel(frameStatsZones);
        value_zoneOuest->setObjectName("value_zoneOuest");
        value_zoneOuest->setStyleSheet(QString::fromUtf8("color: #9b59b6; font-weight: bold; font-size: 12px;"));
        value_zoneOuest->setAlignment(Qt::AlignCenter);

        zoneOuestLayout->addWidget(value_zoneOuest);


        zonesContainerLayout->addLayout(zoneOuestLayout);


        lytStatsZones->addLayout(zonesContainerLayout);

        label_10 = new QLabel(page);
        label_10->setObjectName("label_10");
        label_10->setGeometry(QRect(1150, 90, 61, 21));
        label_10->setAlignment(Qt::AlignmentFlag::AlignCenter);
        leSearch = new QLineEdit(page);
        leSearch->setObjectName("leSearch");
        leSearch->setGeometry(QRect(680, 70, 221, 50));
        leSearch->setAlignment(Qt::AlignmentFlag::AlignCenter);
        frameLeft = new QFrame(page);
        frameLeft->setObjectName("frameLeft");
        frameLeft->setGeometry(QRect(10, 100, 331, 441));
        frameLeft->setMinimumSize(QSize(320, 0));
        frameLeft->setMaximumSize(QSize(360, 16777215));
        frameLeft->setFrameShape(QFrame::Shape::StyledPanel);
        frameLeft->setFrameShadow(QFrame::Shadow::Raised);
        sbQuantite_2 = new QSpinBox(frameLeft);
        sbQuantite_2->setObjectName("sbQuantite_2");
        sbQuantite_2->setGeometry(QRect(81, 200, 211, 51));
        dsPoids_2 = new QDoubleSpinBox(frameLeft);
        dsPoids_2->setObjectName("dsPoids_2");
        dsPoids_2->setGeometry(QRect(80, 260, 211, 51));
        deDateCapture_2 = new QDateEdit(frameLeft);
        deDateCapture_2->setObjectName("deDateCapture_2");
        deDateCapture_2->setGeometry(QRect(80, 320, 211, 51));
        deDateCapture_2->setCalendarPopup(true);
        lineEdit_5 = new QLineEdit(frameLeft);
        lineEdit_5->setObjectName("lineEdit_5");
        lineEdit_5->setGeometry(QRect(10, 10, 281, 50));
        lineEdit_6 = new QLineEdit(frameLeft);
        lineEdit_6->setObjectName("lineEdit_6");
        lineEdit_6->setGeometry(QRect(10, 60, 281, 50));
        label_5 = new QLabel(frameLeft);
        label_5->setObjectName("label_5");
        label_5->setGeometry(QRect(10, 200, 61, 31));
        label_6 = new QLabel(frameLeft);
        label_6->setObjectName("label_6");
        label_6->setGeometry(QRect(20, 270, 61, 31));
        label_7 = new QLabel(frameLeft);
        label_7->setObjectName("label_7");
        label_7->setGeometry(QRect(20, 330, 51, 41));
        btnAffecter_2 = new QPushButton(frameLeft);
        btnAffecter_2->setObjectName("btnAffecter_2");
        btnAffecter_2->setGeometry(QRect(40, 500, 181, 61));
        btnModifier_2 = new QPushButton(frameLeft);
        btnModifier_2->setObjectName("btnModifier_2");
        btnModifier_2->setGeometry(QRect(150, 380, 151, 61));
        btnValiider_2 = new QPushButton(frameLeft);
        btnValiider_2->setObjectName("btnValiider_2");
        btnValiider_2->setGeometry(QRect(10, 380, 131, 61));
        label_8 = new QLabel(frameLeft);
        label_8->setObjectName("label_8");
        label_8->setGeometry(QRect(10, 140, 131, 31));
        comboBox_2 = new QComboBox(frameLeft);
        comboBox_2->addItem(QString());
        comboBox_2->addItem(QString());
        comboBox_2->addItem(QString());
        comboBox_2->addItem(QString());
        comboBox_2->addItem(QString());
        comboBox_2->addItem(QString());
        comboBox_2->addItem(QString());
        comboBox_2->addItem(QString());
        comboBox_2->addItem(QString());
        comboBox_2->addItem(QString());
        comboBox_2->addItem(QString());
        comboBox_2->setObjectName("comboBox_2");
        comboBox_2->setGeometry(QRect(140, 140, 161, 41));
        btnReafraichir = new QPushButton(page);
        btnReafraichir->setObjectName("btnReafraichir");
        btnReafraichir->setGeometry(QRect(1120, 400, 121, 61));
        btnEditLefft = new QPushButton(page);
        btnEditLefft->setObjectName("btnEditLefft");
        btnEditLefft->setGeometry(QRect(950, 410, 121, 61));
        tableWidget_2 = new QTableWidget(page);
        if (tableWidget_2->columnCount() < 1)
            tableWidget_2->setColumnCount(1);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tableWidget_2->setHorizontalHeaderItem(0, __qtablewidgetitem);
        if (tableWidget_2->rowCount() < 9)
            tableWidget_2->setRowCount(9);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tableWidget_2->setVerticalHeaderItem(0, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tableWidget_2->setVerticalHeaderItem(1, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tableWidget_2->setVerticalHeaderItem(2, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        tableWidget_2->setVerticalHeaderItem(3, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        tableWidget_2->setVerticalHeaderItem(4, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        tableWidget_2->setVerticalHeaderItem(5, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        tableWidget_2->setVerticalHeaderItem(6, __qtablewidgetitem7);
        QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
        tableWidget_2->setVerticalHeaderItem(7, __qtablewidgetitem8);
        QTableWidgetItem *__qtablewidgetitem9 = new QTableWidgetItem();
        tableWidget_2->setVerticalHeaderItem(8, __qtablewidgetitem9);
        QTableWidgetItem *__qtablewidgetitem10 = new QTableWidgetItem();
        tableWidget_2->setItem(0, 0, __qtablewidgetitem10);
        tableWidget_2->setObjectName("tableWidget_2");
        tableWidget_2->setGeometry(QRect(370, 120, 241, 391));
        btnExporter_2 = new QPushButton(page);
        btnExporter_2->setObjectName("btnExporter_2");
        btnExporter_2->setGeometry(QRect(20, 730, 121, 71));
        deDebut = new QDateEdit(page);
        deDebut->setObjectName("deDebut");
        deDebut->setGeometry(QRect(1250, 80, 141, 41));
        deDebut->setCalendarPopup(true);
        btnExporter = new QPushButton(page);
        btnExporter->setObjectName("btnExporter");
        btnExporter->setGeometry(QRect(20, 640, 121, 71));
        lblTitle = new QLabel(page);
        lblTitle->setObjectName("lblTitle");
        lblTitle->setGeometry(QRect(480, 0, 421, 51));
        frame = new QFrame(page);
        frame->setObjectName("frame");
        frame->setGeometry(QRect(200, 570, 471, 241));
        frame->setFrameShape(QFrame::Shape::StyledPanel);
        frame->setFrameShadow(QFrame::Shadow::Raised);
        lineEdit_7 = new QLineEdit(frame);
        lineEdit_7->setObjectName("lineEdit_7");
        lineEdit_7->setGeometry(QRect(10, 40, 151, 50));
        lineEdit_8 = new QLineEdit(frame);
        lineEdit_8->setObjectName("lineEdit_8");
        lineEdit_8->setGeometry(QRect(10, 150, 151, 50));
        btnExporter_5 = new QPushButton(frame);
        btnExporter_5->setObjectName("btnExporter_5");
        btnExporter_5->setGeometry(QRect(170, 50, 291, 71));
        btnExporter_6 = new QPushButton(frame);
        btnExporter_6->setObjectName("btnExporter_6");
        btnExporter_6->setGeometry(QRect(170, 160, 291, 71));
        pushButton = new QPushButton(page);
        pushButton->setObjectName("pushButton");
        pushButton->setGeometry(QRect(1270, 400, 141, 61));
        btnAjouter = new QPushButton(page);
        btnAjouter->setObjectName("btnAjouter");
        btnAjouter->setGeometry(QRect(700, 410, 191, 61));
        sbQuantite_3 = new QSpinBox(page);
        sbQuantite_3->setObjectName("sbQuantite_3");
        sbQuantite_3->setGeometry(QRect(1020, 80, 101, 41));
        labelLeftTitle = new QLabel(page);
        labelLeftTitle->setObjectName("labelLeftTitle");
        labelLeftTitle->setGeometry(QRect(30, 60, 101, 31));
        label_11 = new QLabel(page);
        label_11->setObjectName("label_11");
        label_11->setGeometry(QRect(910, 80, 101, 41));
        label_11->setAlignment(Qt::AlignmentFlag::AlignCenter);
        tableWidget = new QTableWidget(page);
        if (tableWidget->columnCount() < 6)
            tableWidget->setColumnCount(6);
        QTableWidgetItem *__qtablewidgetitem11 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(0, __qtablewidgetitem11);
        QTableWidgetItem *__qtablewidgetitem12 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(1, __qtablewidgetitem12);
        QTableWidgetItem *__qtablewidgetitem13 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(2, __qtablewidgetitem13);
        QTableWidgetItem *__qtablewidgetitem14 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(3, __qtablewidgetitem14);
        QTableWidgetItem *__qtablewidgetitem15 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(4, __qtablewidgetitem15);
        QTableWidgetItem *__qtablewidgetitem16 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(5, __qtablewidgetitem16);
        tableWidget->setObjectName("tableWidget");
        tableWidget->setGeometry(QRect(660, 130, 771, 351));
        stackedWidget->addWidget(page);
        label_10->raise();
        leSearch->raise();
        frameLeft->raise();
        tableWidget_2->raise();
        btnExporter_2->raise();
        deDebut->raise();
        btnExporter->raise();
        lblTitle->raise();
        frame->raise();
        sbQuantite_3->raise();
        labelLeftTitle->raise();
        label_11->raise();
        tableWidget->raise();
        btnEditLefft->raise();
        btnReafraichir->raise();
        pushButton->raise();
        btnAjouter->raise();
        page_3 = new QWidget();
        page_3->setObjectName("page_3");
        lblTitle_3 = new QLabel(page_3);
        lblTitle_3->setObjectName("lblTitle_3");
        lblTitle_3->setGeometry(QRect(460, 20, 561, 61));
        frame_3 = new QFrame(page_3);
        frame_3->setObjectName("frame_3");
        frame_3->setGeometry(QRect(80, 110, 1121, 631));
        frame_3->setFrameShape(QFrame::Shape::StyledPanel);
        frame_3->setFrameShadow(QFrame::Shadow::Raised);
        comboBox_3 = new QComboBox(frame_3);
        comboBox_3->setObjectName("comboBox_3");
        comboBox_3->setGeometry(QRect(360, 20, 171, 41));
        label_27 = new QLabel(frame_3);
        label_27->setObjectName("label_27");
        label_27->setGeometry(QRect(30, 10, 91, 61));
        label_28 = new QLabel(frame_3);
        label_28->setObjectName("label_28");
        label_28->setGeometry(QRect(540, 10, 111, 61));
        dateEdit = new QDateEdit(frame_3);
        dateEdit->setObjectName("dateEdit");
        dateEdit->setGeometry(QRect(650, 20, 121, 51));
        label_29 = new QLabel(frame_3);
        label_29->setObjectName("label_29");
        label_29->setGeometry(QRect(780, 20, 21, 41));
        dateEdit_2 = new QDateEdit(frame_3);
        dateEdit_2->setObjectName("dateEdit_2");
        dateEdit_2->setGeometry(QRect(810, 20, 121, 51));
        pushButton_4 = new QPushButton(frame_3);
        pushButton_4->setObjectName("pushButton_4");
        pushButton_4->setGeometry(QRect(950, 20, 161, 61));
        label_30 = new QLabel(frame_3);
        label_30->setObjectName("label_30");
        label_30->setGeometry(QRect(280, 10, 91, 61));
        groupStatsBateau = new QGroupBox(frame_3);
        groupStatsBateau->setObjectName("groupStatsBateau");
        groupStatsBateau->setGeometry(QRect(70, 120, 951, 371));
        label_idBateau = new QLabel(groupStatsBateau);
        label_idBateau->setObjectName("label_idBateau");
        label_idBateau->setGeometry(QRect(12, 31, 92, 20));
        label_idBateau->setStyleSheet(QString::fromUtf8("font-weight: bold; color: white;"));
        lblIdBateauValue = new QLabel(groupStatsBateau);
        lblIdBateauValue->setObjectName("lblIdBateauValue");
        lblIdBateauValue->setGeometry(QRect(111, 31, 171, 21));
        lblIdBateauValue->setStyleSheet(QString::fromUtf8("color: #59abc8; font-weight: bold; font-size: 16px;"));
        label_31 = new QLabel(frame_3);
        label_31->setObjectName("label_31");
        label_31->setGeometry(QRect(210, 520, 51, 61));
        label_32 = new QLabel(frame_3);
        label_32->setObjectName("label_32");
        label_32->setGeometry(QRect(60, 520, 131, 61));
        label_33 = new QLabel(frame_3);
        label_33->setObjectName("label_33");
        label_33->setGeometry(QRect(420, 520, 151, 61));
        label_34 = new QLabel(frame_3);
        label_34->setObjectName("label_34");
        label_34->setGeometry(QRect(580, 520, 51, 61));
        pushButton_5 = new QPushButton(page_3);
        pushButton_5->setObjectName("pushButton_5");
        pushButton_5->setGeometry(QRect(620, 770, 191, 61));
        stackedWidget->addWidget(page_3);
        page_2 = new QWidget();
        page_2->setObjectName("page_2");
        lblTitle_2 = new QLabel(page_2);
        lblTitle_2->setObjectName("lblTitle_2");
        lblTitle_2->setGeometry(QRect(460, 10, 361, 71));
        lblTitle_2->setStyleSheet(QString::fromUtf8("\n"
"font: 900 italic 12pt \"Segoe UI\";"));
        lblTitle_2->setAlignment(Qt::AlignmentFlag::AlignCenter);
        frame_2 = new QFrame(page_2);
        frame_2->setObjectName("frame_2");
        frame_2->setGeometry(QRect(150, 90, 951, 721));
        frame_2->setFrameShape(QFrame::Shape::StyledPanel);
        frame_2->setFrameShadow(QFrame::Shadow::Raised);
        label_19 = new QLabel(frame_2);
        label_19->setObjectName("label_19");
        label_19->setGeometry(QRect(20, 0, 101, 41));
        label_20 = new QLabel(frame_2);
        label_20->setObjectName("label_20");
        label_20->setGeometry(QRect(480, 0, 51, 41));
        label_21 = new QLabel(frame_2);
        label_21->setObjectName("label_21");
        label_21->setGeometry(QRect(660, 0, 241, 41));
        tableWidget_4 = new QTableWidget(frame_2);
        if (tableWidget_4->columnCount() < 6)
            tableWidget_4->setColumnCount(6);
        QTableWidgetItem *__qtablewidgetitem17 = new QTableWidgetItem();
        tableWidget_4->setHorizontalHeaderItem(0, __qtablewidgetitem17);
        QTableWidgetItem *__qtablewidgetitem18 = new QTableWidgetItem();
        tableWidget_4->setHorizontalHeaderItem(1, __qtablewidgetitem18);
        QTableWidgetItem *__qtablewidgetitem19 = new QTableWidgetItem();
        tableWidget_4->setHorizontalHeaderItem(2, __qtablewidgetitem19);
        QTableWidgetItem *__qtablewidgetitem20 = new QTableWidgetItem();
        tableWidget_4->setHorizontalHeaderItem(3, __qtablewidgetitem20);
        QTableWidgetItem *__qtablewidgetitem21 = new QTableWidgetItem();
        tableWidget_4->setHorizontalHeaderItem(4, __qtablewidgetitem21);
        QTableWidgetItem *__qtablewidgetitem22 = new QTableWidgetItem();
        tableWidget_4->setHorizontalHeaderItem(5, __qtablewidgetitem22);
        tableWidget_4->setObjectName("tableWidget_4");
        tableWidget_4->setGeometry(QRect(80, 70, 761, 311));
        label_22 = new QLabel(frame_2);
        label_22->setObjectName("label_22");
        label_22->setGeometry(QRect(240, 0, 81, 41));
        groupBox_3 = new QGroupBox(frame_2);
        groupBox_3->setObjectName("groupBox_3");
        groupBox_3->setGeometry(QRect(30, 400, 531, 241));
        label_23 = new QLabel(groupBox_3);
        label_23->setObjectName("label_23");
        label_23->setGeometry(QRect(150, 30, 171, 31));
        textBrowser_2 = new QTextBrowser(groupBox_3);
        textBrowser_2->setObjectName("textBrowser_2");
        textBrowser_2->setGeometry(QRect(10, 70, 471, 161));
        groupBox_4 = new QGroupBox(frame_2);
        groupBox_4->setObjectName("groupBox_4");
        groupBox_4->setGeometry(QRect(620, 400, 311, 241));
        progressBar_4 = new QProgressBar(groupBox_4);
        progressBar_4->setObjectName("progressBar_4");
        progressBar_4->setGeometry(QRect(160, 70, 131, 21));
        progressBar_4->setValue(24);
        progressBar_5 = new QProgressBar(groupBox_4);
        progressBar_5->setObjectName("progressBar_5");
        progressBar_5->setGeometry(QRect(160, 120, 131, 21));
        progressBar_5->setValue(24);
        progressBar_6 = new QProgressBar(groupBox_4);
        progressBar_6->setObjectName("progressBar_6");
        progressBar_6->setGeometry(QRect(160, 160, 131, 21));
        progressBar_6->setValue(24);
        label_24 = new QLabel(groupBox_4);
        label_24->setObjectName("label_24");
        label_24->setGeometry(QRect(10, 70, 141, 21));
        label_25 = new QLabel(groupBox_4);
        label_25->setObjectName("label_25");
        label_25->setGeometry(QRect(10, 120, 141, 21));
        label_26 = new QLabel(groupBox_4);
        label_26->setObjectName("label_26");
        label_26->setGeometry(QRect(10, 160, 141, 21));
        pushButton_3 = new QPushButton(frame_2);
        pushButton_3->setObjectName("pushButton_3");
        pushButton_3->setGeometry(QRect(410, 660, 171, 61));
        stackedWidget->addWidget(page_2);
        Gcaptures->setCentralWidget(centralwidget);
        menubar = new QMenuBar(Gcaptures);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1476, 25));
        Gcaptures->setMenuBar(menubar);
        statusbar = new QStatusBar(Gcaptures);
        statusbar->setObjectName("statusbar");
        Gcaptures->setStatusBar(statusbar);

        retranslateUi(Gcaptures);

        QMetaObject::connectSlotsByName(Gcaptures);
    } // setupUi

    void retranslateUi(QMainWindow *Gcaptures)
    {
        Gcaptures->setWindowTitle(QCoreApplication::translate("Gcaptures", "GestionCaptures", nullptr));
        label_zones->setText(QCoreApplication::translate("Gcaptures", "R\303\211PARTITION PAR ZONE :", nullptr));
        label_zoneNord->setText(QCoreApplication::translate("Gcaptures", "Nord", nullptr));
        progressZoneNord->setFormat(QCoreApplication::translate("Gcaptures", "%p%", nullptr));
        value_zoneNord->setText(QCoreApplication::translate("Gcaptures", "0/0", nullptr));
        label_zoneSud->setText(QCoreApplication::translate("Gcaptures", "Sud", nullptr));
        progressZoneSud->setFormat(QCoreApplication::translate("Gcaptures", "%p%", nullptr));
        value_zoneSud->setText(QCoreApplication::translate("Gcaptures", "0/0", nullptr));
        label_zoneEst->setText(QCoreApplication::translate("Gcaptures", "Est", nullptr));
        progressZoneEst->setFormat(QCoreApplication::translate("Gcaptures", "%p%", nullptr));
        value_zoneEst->setText(QCoreApplication::translate("Gcaptures", "0/0", nullptr));
        label_zoneOuest->setText(QCoreApplication::translate("Gcaptures", "Ouest", nullptr));
        progressZoneOuest->setFormat(QCoreApplication::translate("Gcaptures", "%p%", nullptr));
        value_zoneOuest->setText(QCoreApplication::translate("Gcaptures", "0/0", nullptr));
        label_10->setText(QCoreApplication::translate("Gcaptures", "DATE :", nullptr));
        leSearch->setText(QCoreApplication::translate("Gcaptures", "search par ID capture", nullptr));
        lineEdit_5->setPlaceholderText(QCoreApplication::translate("Gcaptures", "ID Capture", nullptr));
        lineEdit_6->setPlaceholderText(QCoreApplication::translate("Gcaptures", "ID Bateau", nullptr));
        label_5->setText(QCoreApplication::translate("Gcaptures", "Quantit\303\251 ", nullptr));
        label_6->setText(QCoreApplication::translate("Gcaptures", "POIDS", nullptr));
        label_7->setText(QCoreApplication::translate("Gcaptures", "DATE", nullptr));
        btnAffecter_2->setText(QCoreApplication::translate("Gcaptures", "Affecter Bateau", nullptr));
        btnModifier_2->setText(QCoreApplication::translate("Gcaptures", "Modifier", nullptr));
        btnValiider_2->setText(QCoreApplication::translate("Gcaptures", "Valider", nullptr));
        label_8->setText(QCoreApplication::translate("Gcaptures", "Types de Poisson", nullptr));
        comboBox_2->setItemText(0, QCoreApplication::translate("Gcaptures", "SARDINES", nullptr));
        comboBox_2->setItemText(1, QCoreApplication::translate("Gcaptures", "THON", nullptr));
        comboBox_2->setItemText(2, QCoreApplication::translate("Gcaptures", "MERLU", nullptr));
        comboBox_2->setItemText(3, QCoreApplication::translate("Gcaptures", "DORADE", nullptr));
        comboBox_2->setItemText(4, QCoreApplication::translate("Gcaptures", "Loup", nullptr));
        comboBox_2->setItemText(5, QCoreApplication::translate("Gcaptures", "ROUGET", nullptr));
        comboBox_2->setItemText(6, QCoreApplication::translate("Gcaptures", "CALAMAR", nullptr));
        comboBox_2->setItemText(7, QCoreApplication::translate("Gcaptures", "POULPES", nullptr));
        comboBox_2->setItemText(8, QCoreApplication::translate("Gcaptures", "CREVETTE", nullptr));
        comboBox_2->setItemText(9, QCoreApplication::translate("Gcaptures", "MEROU", nullptr));
        comboBox_2->setItemText(10, QCoreApplication::translate("Gcaptures", "MAQUEREAU", nullptr));

        btnReafraichir->setText(QCoreApplication::translate("Gcaptures", "Reafraichir", nullptr));
        btnEditLefft->setText(QCoreApplication::translate("Gcaptures", "Modifier", nullptr));
        QTableWidgetItem *___qtablewidgetitem = tableWidget_2->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("Gcaptures", "QUOTAS", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = tableWidget_2->verticalHeaderItem(0);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("Gcaptures", "SARDINE", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = tableWidget_2->verticalHeaderItem(1);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("Gcaptures", "MAQUEREAU", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = tableWidget_2->verticalHeaderItem(2);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("Gcaptures", "MERLU", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = tableWidget_2->verticalHeaderItem(3);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("Gcaptures", "THON", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = tableWidget_2->verticalHeaderItem(4);
        ___qtablewidgetitem5->setText(QCoreApplication::translate("Gcaptures", "LOUP", nullptr));
        QTableWidgetItem *___qtablewidgetitem6 = tableWidget_2->verticalHeaderItem(5);
        ___qtablewidgetitem6->setText(QCoreApplication::translate("Gcaptures", "CALAMR", nullptr));
        QTableWidgetItem *___qtablewidgetitem7 = tableWidget_2->verticalHeaderItem(6);
        ___qtablewidgetitem7->setText(QCoreApplication::translate("Gcaptures", "CREVETTE", nullptr));
        QTableWidgetItem *___qtablewidgetitem8 = tableWidget_2->verticalHeaderItem(7);
        ___qtablewidgetitem8->setText(QCoreApplication::translate("Gcaptures", "ROUGET", nullptr));
        QTableWidgetItem *___qtablewidgetitem9 = tableWidget_2->verticalHeaderItem(8);
        ___qtablewidgetitem9->setText(QCoreApplication::translate("Gcaptures", "POULPES", nullptr));

        const bool __sortingEnabled = tableWidget_2->isSortingEnabled();
        tableWidget_2->setSortingEnabled(false);
        QTableWidgetItem *___qtablewidgetitem10 = tableWidget_2->item(0, 0);
        ___qtablewidgetitem10->setText(QCoreApplication::translate("Gcaptures", "200kg", nullptr));
        tableWidget_2->setSortingEnabled(__sortingEnabled);

        btnExporter_2->setText(QCoreApplication::translate("Gcaptures", "Affecter", nullptr));
        btnExporter->setText(QCoreApplication::translate("Gcaptures", "Exporter", nullptr));
        lblTitle->setText(QCoreApplication::translate("Gcaptures", "Gestion Des Captures", nullptr));
        lineEdit_7->setPlaceholderText(QCoreApplication::translate("Gcaptures", "ID Capture", nullptr));
        lineEdit_8->setPlaceholderText(QCoreApplication::translate("Gcaptures", "ID Bateau", nullptr));
        btnExporter_5->setText(QCoreApplication::translate("Gcaptures", "VERIFIER SURPECHE", nullptr));
        btnExporter_6->setText(QCoreApplication::translate("Gcaptures", "VERIFIER TENDANCE SAISONIERE", nullptr));
        pushButton->setText(QCoreApplication::translate("Gcaptures", "Supprimer", nullptr));
        btnAjouter->setText(QCoreApplication::translate("Gcaptures", "Ajouter", nullptr));
        labelLeftTitle->setText(QCoreApplication::translate("Gcaptures", "Formulaire", nullptr));
        label_11->setText(QCoreApplication::translate("Gcaptures", "QUANTITE", nullptr));
        QTableWidgetItem *___qtablewidgetitem11 = tableWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem11->setText(QCoreApplication::translate("Gcaptures", "ID CAPTURES", nullptr));
        QTableWidgetItem *___qtablewidgetitem12 = tableWidget->horizontalHeaderItem(1);
        ___qtablewidgetitem12->setText(QCoreApplication::translate("Gcaptures", "ID Bateau", nullptr));
        QTableWidgetItem *___qtablewidgetitem13 = tableWidget->horizontalHeaderItem(2);
        ___qtablewidgetitem13->setText(QCoreApplication::translate("Gcaptures", "Types Poisson", nullptr));
        QTableWidgetItem *___qtablewidgetitem14 = tableWidget->horizontalHeaderItem(3);
        ___qtablewidgetitem14->setText(QCoreApplication::translate("Gcaptures", "Quantit\303\251s", nullptr));
        QTableWidgetItem *___qtablewidgetitem15 = tableWidget->horizontalHeaderItem(4);
        ___qtablewidgetitem15->setText(QCoreApplication::translate("Gcaptures", "Poids", nullptr));
        QTableWidgetItem *___qtablewidgetitem16 = tableWidget->horizontalHeaderItem(5);
        ___qtablewidgetitem16->setText(QCoreApplication::translate("Gcaptures", "Dates", nullptr));
        lblTitle_3->setText(QCoreApplication::translate("Gcaptures", "TENDANCE SAISONIERE ", nullptr));
        label_27->setText(QCoreApplication::translate("Gcaptures", "ID BATEAU :", nullptr));
        label_28->setText(QCoreApplication::translate("Gcaptures", "PERIODE : du", nullptr));
        label_29->setText(QCoreApplication::translate("Gcaptures", "au", nullptr));
        pushButton_4->setText(QCoreApplication::translate("Gcaptures", "AFFICHER", nullptr));
        label_30->setText(QCoreApplication::translate("Gcaptures", "ESPECE :", nullptr));
        groupStatsBateau->setTitle(QCoreApplication::translate("Gcaptures", "Stats Bateau", nullptr));
        label_idBateau->setText(QCoreApplication::translate("Gcaptures", "ID BATEAU :", nullptr));
        lblIdBateauValue->setText(QCoreApplication::translate("Gcaptures", "---", nullptr));
        label_31->setText(QCoreApplication::translate("Gcaptures", "--KG", nullptr));
        label_32->setText(QCoreApplication::translate("Gcaptures", "POIDS TOTALE :", nullptr));
        label_33->setText(QCoreApplication::translate("Gcaptures", "QUANTITE TOTALE :", nullptr));
        label_34->setText(QCoreApplication::translate("Gcaptures", "--", nullptr));
        pushButton_5->setText(QCoreApplication::translate("Gcaptures", "FERMER", nullptr));
        lblTitle_2->setText(QCoreApplication::translate("Gcaptures", "Analyse de Surp\303\252che", nullptr));
        label_19->setText(QCoreApplication::translate("Gcaptures", "ID CAPTURE :", nullptr));
        label_20->setText(QCoreApplication::translate("Gcaptures", "DATE :", nullptr));
        label_21->setText(QCoreApplication::translate("Gcaptures", "NOMBRE D ESPECE CAPTURE :", nullptr));
        QTableWidgetItem *___qtablewidgetitem17 = tableWidget_4->horizontalHeaderItem(0);
        ___qtablewidgetitem17->setText(QCoreApplication::translate("Gcaptures", "Esp\303\251ce", nullptr));
        QTableWidgetItem *___qtablewidgetitem18 = tableWidget_4->horizontalHeaderItem(1);
        ___qtablewidgetitem18->setText(QCoreApplication::translate("Gcaptures", "Quantit\303\251", nullptr));
        QTableWidgetItem *___qtablewidgetitem19 = tableWidget_4->horizontalHeaderItem(2);
        ___qtablewidgetitem19->setText(QCoreApplication::translate("Gcaptures", "Poids", nullptr));
        QTableWidgetItem *___qtablewidgetitem20 = tableWidget_4->horizontalHeaderItem(3);
        ___qtablewidgetitem20->setText(QCoreApplication::translate("Gcaptures", "Quotas", nullptr));
        QTableWidgetItem *___qtablewidgetitem21 = tableWidget_4->horizontalHeaderItem(4);
        ___qtablewidgetitem21->setText(QCoreApplication::translate("Gcaptures", "% Utilis\303\251", nullptr));
        QTableWidgetItem *___qtablewidgetitem22 = tableWidget_4->horizontalHeaderItem(5);
        ___qtablewidgetitem22->setText(QCoreApplication::translate("Gcaptures", "Etat", nullptr));
        label_22->setText(QCoreApplication::translate("Gcaptures", "BATEAU :", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("Gcaptures", "DIAGNOSTIC", nullptr));
        label_23->setText(QCoreApplication::translate("Gcaptures", "\303\211tat Globale : ---", nullptr));
        textBrowser_2->setHtml(QCoreApplication::translate("Gcaptures", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'Segoe UI','Century Gothic','sans-serif'; font-size:14px; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Segoe UI'; font-size:9pt; color:#ffffff;\">Analyse en attente...</span></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'Segoe UI'; font-size:9pt; color:#ffffff;\"><br /></p></body></html>", nullptr));
        groupBox_4->setTitle(QCoreApplication::translate("Gcaptures", "INDICATEURS", nullptr));
        label_24->setText(QCoreApplication::translate("Gcaptures", "NOM ESPECE : ---", nullptr));
        label_25->setText(QCoreApplication::translate("Gcaptures", "NOM ESPECE : ---", nullptr));
        label_26->setText(QCoreApplication::translate("Gcaptures", "NOM ESPECE : ---", nullptr));
        pushButton_3->setText(QCoreApplication::translate("Gcaptures", "FERMER", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Gcaptures: public Ui_Gcaptures {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GCAPTURES_H
