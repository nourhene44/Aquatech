/********************************************************************************
** Form generated from reading UI file 'gpecheurs.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GPECHEURS_H
#define UI_GPECHEURS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Gpecheurs
{
public:
    QWidget *centralwidget;
    QStackedWidget *stackedWidget;
    QWidget *pagepecheur;
    QLabel *labelp;
    QFrame *framep;
    QLineEdit *lineEditp;
    QLineEdit *lineEdit_2p;
    QComboBox *comboBoxp;
    QComboBox *comboBox_2;
    QComboBox *comboBox_3p;
    QLineEdit *lineEdit_3p;
    QPushButton *btnFaceIDp;
    QPushButton *bmip;
    QPushButton *bap;
    QLabel *label_5p;
    QLabel *label_6p;
    QLabel *label_7p;
    QTableWidget *tableWidgetp;
    QComboBox *comboBox_5p;
    QLineEdit *lineEdit_4p;
    QLabel *label_3p;
    QDialog *dialogFaceIDp;
    QVBoxLayout *verticalLayoutFaceID;
    QLabel *labelFaceTitlep;
    QLabel *labelCamerap;
    QLabel *labelFaceStatusp;
    QHBoxLayout *horizontalLayoutFaceButtonsp;
    QPushButton *btnCapturep;
    QPushButton *btnVerifyp;
    QPushButton *btnCancelFacep;
    QLabel *labelFaceInfop;
    QPushButton *pushButton_4p;
    QPushButton *pushButton_5p;
    QPushButton *pushButton_6p;
    QFrame *frame_typesp;
    QVBoxLayout *verticalLayout_types;
    QLabel *label_typesp;
    QHBoxLayout *horizontalLayout_typesp;
    QLabel *progressTypeCirclep;
    QVBoxLayout *verticalLayout_legendp;
    QLabel *label_total_typesp;
    QLabel *label_legend_chalutierp;
    QLabel *label_legend_palangrierp;
    QLabel *label_legend_caseyeurp;
    QLabel *label_legend_traditionalp;
    QLabel *label_legend_otherp;
    QLabel *labelfp;
    QLabel *label_2p;
    QComboBox *comboBox_6p;
    QLabel *label_4p;
    QPushButton *brmp;
    QPushButton *bep;
    QFrame *framefaceidp;
    QLabel *faceGuideCircle_4p;
    QLabel *label_zones_4p;
    QFrame *frame_10p;
    QLabel *label_legend_chalutier_11p;
    QLabel *label_legend_chalutier_12p;
    QFrame *frame_11p;
    QLabel *label_legend_chalutier_13p;
    QLineEdit *lineEdit_11p;
    QLineEdit *lineEdit_12p;
    QPushButton *pushButton_10p;
    QPushButton *pushButton_11p;
    QPushButton *bmi_6p;
    QLabel *label_8p;
    QWidget *page5;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *Gpecheurs)
    {
        if (Gpecheurs->objectName().isEmpty())
            Gpecheurs->setObjectName("Gpecheurs");
        Gpecheurs->resize(1512, 943);
        centralwidget = new QWidget(Gpecheurs);
        centralwidget->setObjectName("centralwidget");
        centralwidget->setEnabled(true);
        stackedWidget = new QStackedWidget(centralwidget);
        stackedWidget->setObjectName("stackedWidget");
        stackedWidget->setEnabled(true);
        stackedWidget->setGeometry(QRect(-10, -50, 1511, 911));
        stackedWidget->setStyleSheet(QString::fromUtf8("#page5{\n"
"background-image: url(:/bg.png);\n"
"background-repeat: no-repeat;\n"
"    background-position: center;\n"
"}\n"
"#pagepecheur {\n"
"   background-image: url(:/bg.png);\n"
"    background-repeat: no-repeat;\n"
"    background-position: center;\n"
"border-radius:15px\n"
"}\n"
"#labelfp{\n"
"color:#fffae0;\n"
"    font-weight: 600;\n"
"    font-size: 17px;\n"
"    padding: 5px 0;\n"
"}\n"
"\n"
"QComboBox {\n"
"font-size: 16px;\n"
"    padding: 6px;\n"
"    border: 2px solid #59abc8;\n"
"    border-radius: 15px;\n"
"        background-color: rgba(0, 0, 50, 0.8);\n"
"\n"
"\n"
"}\n"
"\n"
"QLineEdit {\n"
"font-size: 17px;\n"
"    background: transparent;\n"
"    border: none;\n"
"    color: WHITE;\n"
"    border-bottom: 2px solid #0078D7;\n"
"    padding: 3px;\n"
"    selection-background-color: #FF8C00;\n"
"}\n"
"#lineEdit_4p{\n"
"  font-size: 17px;\n"
"    background: transparent;\n"
"    border: none;\n"
"    color: black;\n"
"    border-bottom: 2px solid #0078D7;\n"
"    padding: 3px;\n"
"    selecti"
                        "on-background-color: #FF8C00;\n"
"}\n"
"\n"
"\n"
"QPushButton {\n"
"    border-radius: 20px;\n"
"background-color: rgba(0, 0, 150, 0.7);\n"
"   border: 2px solid #59abc8;\n"
" color: white;\n"
"    font-weight: 700;\n"
"    font-size: 15px;\n"
"    margin-bottom:20px;\n"
"    text-shadow: 0 1px 2px rgba(0, 0, 0, 0.3);\n"
"padding: 10px 20px;\n"
"}\n"
"QPushButton:hover {\n"
"    transform: translateY(-2px);\n"
"    color: #59abc8;\n"
"}\n"
"#pushButtonp:hover{\n"
"transform: translateY(-2px);\n"
"    color: #80000;\n"
"}\n"
"\n"
"#labelp{/* Labels sp\303\251ciaux */\n"
"    border:none;\n"
"    background-color: transparent;\n"
"    font-weight: 700;\n"
"color: rgb(66, 157, 255);\n"
"font: 25pt \"Imprint MT Shadow\";\n"
"}\n"
"#label_2p,#label_4p,#label_5p,#label_6p,#label_7p,#label_3p{\n"
"    border:none;\n"
"    background-color:transparent;\n"
"    font-size:20px;\n"
"    color:#fffff;\n"
"        color: rgb(255, 255, 255);\n"
"    font-weight:500;\n"
"    font-family: \"Segoe UI\", \"Century Gothic\", san"
                        "s-serif;\n"
"}\n"
"QFrame {\n"
"\n"
"     border-radius: 15px;\n"
"        background-color: rgba(0, 0, 50, 0.7);\n"
"     border: 2px solid #59abc8 ;\n"
"}\n"
"#frame_3p{\n"
"\n"
"     border-radius: 15px;\n"
"        background-color: rgba(0, 0, 50,0.9);\n"
"     border: 2px solid #59abc8 ;\n"
"}\n"
"#framefaceidp{\n"
"background-color: rgba(0, 0, 50, 0.9);\n"
"}\n"
"\n"
"#btnFaceIDp {\n"
"\n"
"    background-color: rgba(175, 109, 202, 0.7);\n"
"     border-radius: 20px;\n"
"     border: 2px solid #59abc8;\n"
"    color: white;\n"
"    font-weight: 700;\n"
"    font-size: 15px;\n"
"    margin-bottom:20px;\n"
"    text-shadow: 0 1px 2px rgba(0, 0, 0, 0.3);\n"
"padding: 10px 20px;\n"
"}\n"
"#btnFaceIDp:hover {\n"
"    transform: translateY(-2px);\n"
"    color: #9b59b6;\n"
"}\n"
"\n"
""));
        pagepecheur = new QWidget();
        pagepecheur->setObjectName("pagepecheur");
        pagepecheur->setContextMenuPolicy(Qt::ContextMenuPolicy::DefaultContextMenu);
        labelp = new QLabel(pagepecheur);
        labelp->setObjectName("labelp");
        labelp->setEnabled(false);
        labelp->setGeometry(QRect(540, 50, 411, 41));
        framep = new QFrame(pagepecheur);
        framep->setObjectName("framep");
        framep->setEnabled(true);
        framep->setGeometry(QRect(10, 130, 331, 531));
        framep->setFrameShape(QFrame::Shape::StyledPanel);
        framep->setFrameShadow(QFrame::Shadow::Raised);
        lineEditp = new QLineEdit(framep);
        lineEditp->setObjectName("lineEditp");
        lineEditp->setGeometry(QRect(40, 50, 231, 31));
        lineEdit_2p = new QLineEdit(framep);
        lineEdit_2p->setObjectName("lineEdit_2p");
        lineEdit_2p->setGeometry(QRect(40, 110, 231, 31));
        comboBoxp = new QComboBox(framep);
        comboBoxp->addItem(QString());
        comboBoxp->addItem(QString());
        comboBoxp->addItem(QString());
        comboBoxp->setObjectName("comboBoxp");
        comboBoxp->setGeometry(QRect(130, 230, 151, 41));
        comboBoxp->setEditable(true);
        comboBox_2 = new QComboBox(framep);
        comboBox_2->addItem(QString());
        comboBox_2->addItem(QString());
        comboBox_2->setObjectName("comboBox_2");
        comboBox_2->setGeometry(QRect(130, 290, 151, 41));
        comboBox_2->setEditable(true);
        comboBox_3p = new QComboBox(framep);
        comboBox_3p->addItem(QString());
        comboBox_3p->setObjectName("comboBox_3p");
        comboBox_3p->setGeometry(QRect(120, 350, 151, 41));
        lineEdit_3p = new QLineEdit(framep);
        lineEdit_3p->setObjectName("lineEdit_3p");
        lineEdit_3p->setGeometry(QRect(40, 170, 231, 28));
        btnFaceIDp = new QPushButton(framep);
        btnFaceIDp->setObjectName("btnFaceIDp");
        btnFaceIDp->setGeometry(QRect(50, 410, 201, 61));
        btnFaceIDp->setStyleSheet(QString::fromUtf8(""));
        bmip = new QPushButton(framep);
        bmip->setObjectName("bmip");
        bmip->setGeometry(QRect(170, 470, 121, 61));
        bmip->setStyleSheet(QString::fromUtf8("background-color:rgba(0, 100, 0, 0.7);"));
        bap = new QPushButton(framep);
        bap->setObjectName("bap");
        bap->setGeometry(QRect(30, 470, 101, 61));
        label_5p = new QLabel(framep);
        label_5p->setObjectName("label_5p");
        label_5p->setGeometry(QRect(10, 240, 63, 20));
        label_6p = new QLabel(framep);
        label_6p->setObjectName("label_6p");
        label_6p->setGeometry(QRect(10, 300, 121, 20));
        label_7p = new QLabel(framep);
        label_7p->setObjectName("label_7p");
        label_7p->setGeometry(QRect(10, 360, 81, 20));
        tableWidgetp = new QTableWidget(pagepecheur);
        if (tableWidgetp->columnCount() < 11)
            tableWidgetp->setColumnCount(11);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tableWidgetp->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tableWidgetp->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tableWidgetp->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tableWidgetp->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        tableWidgetp->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        tableWidgetp->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        tableWidgetp->setHorizontalHeaderItem(6, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        tableWidgetp->setHorizontalHeaderItem(7, __qtablewidgetitem7);
        QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
        tableWidgetp->setHorizontalHeaderItem(8, __qtablewidgetitem8);
        QTableWidgetItem *__qtablewidgetitem9 = new QTableWidgetItem();
        tableWidgetp->setHorizontalHeaderItem(9, __qtablewidgetitem9);
        QTableWidgetItem *__qtablewidgetitem10 = new QTableWidgetItem();
        tableWidgetp->setHorizontalHeaderItem(10, __qtablewidgetitem10);
        tableWidgetp->setObjectName("tableWidgetp");
        tableWidgetp->setGeometry(QRect(360, 180, 1111, 351));
        tableWidgetp->setStyleSheet(QString::fromUtf8("QTableWidget {\n"
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
"}"));
        tableWidgetp->horizontalHeader()->setDefaultSectionSize(100);
        comboBox_5p = new QComboBox(pagepecheur);
        comboBox_5p->addItem(QString());
        comboBox_5p->addItem(QString());
        comboBox_5p->addItem(QString());
        comboBox_5p->addItem(QString());
        comboBox_5p->addItem(QString());
        comboBox_5p->addItem(QString());
        comboBox_5p->setObjectName("comboBox_5p");
        comboBox_5p->setGeometry(QRect(670, 120, 161, 41));
        comboBox_5p->setEditable(true);
        lineEdit_4p = new QLineEdit(pagepecheur);
        lineEdit_4p->setObjectName("lineEdit_4p");
        lineEdit_4p->setGeometry(QRect(400, 130, 141, 31));
        label_3p = new QLabel(pagepecheur);
        label_3p->setObjectName("label_3p");
        label_3p->setGeometry(QRect(370, 140, 31, 20));
        dialogFaceIDp = new QDialog(pagepecheur);
        dialogFaceIDp->setObjectName("dialogFaceIDp");
        dialogFaceIDp->setGeometry(QRect(500, 200, 500, 400));
        dialogFaceIDp->setVisible(false);
        verticalLayoutFaceID = new QVBoxLayout(dialogFaceIDp);
        verticalLayoutFaceID->setSpacing(15);
        verticalLayoutFaceID->setObjectName("verticalLayoutFaceID");
        verticalLayoutFaceID->setContentsMargins(20, 20, 20, 20);
        labelFaceTitlep = new QLabel(dialogFaceIDp);
        labelFaceTitlep->setObjectName("labelFaceTitlep");
        labelFaceTitlep->setStyleSheet(QString::fromUtf8("color: #9b59b6;\n"
"font-size: 18px;\n"
"font-weight: bold;\n"
"text-align: center;\n"
"padding: 10px;"));
        labelFaceTitlep->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayoutFaceID->addWidget(labelFaceTitlep);

        labelCamerap = new QLabel(dialogFaceIDp);
        labelCamerap->setObjectName("labelCamerap");
        labelCamerap->setMinimumSize(QSize(320, 240));
        labelCamerap->setStyleSheet(QString::fromUtf8("background-color: black;\n"
"border: 2px solid #3498db;\n"
"border-radius: 10px;"));
        labelCamerap->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayoutFaceID->addWidget(labelCamerap);

        labelFaceStatusp = new QLabel(dialogFaceIDp);
        labelFaceStatusp->setObjectName("labelFaceStatusp");
        labelFaceStatusp->setStyleSheet(QString::fromUtf8("color: #f39c12;\n"
"font-size: 14px;\n"
"text-align: center;\n"
"padding: 10px;"));
        labelFaceStatusp->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayoutFaceID->addWidget(labelFaceStatusp);

        horizontalLayoutFaceButtonsp = new QHBoxLayout();
        horizontalLayoutFaceButtonsp->setSpacing(20);
        horizontalLayoutFaceButtonsp->setObjectName("horizontalLayoutFaceButtonsp");
        btnCapturep = new QPushButton(dialogFaceIDp);
        btnCapturep->setObjectName("btnCapturep");
        btnCapturep->setStyleSheet(QString::fromUtf8("background-color: #3498db;\n"
"border: 2px solid #2980b9;\n"
"color: white;\n"
"padding: 10px;\n"
"font-weight: bold;\n"
"border-radius: 8px;\n"
"font-size: 14px;"));

        horizontalLayoutFaceButtonsp->addWidget(btnCapturep);

        btnVerifyp = new QPushButton(dialogFaceIDp);
        btnVerifyp->setObjectName("btnVerifyp");
        btnVerifyp->setStyleSheet(QString::fromUtf8("background-color: #27ae60;\n"
"border: 2px solid #229954;\n"
"color: white;\n"
"padding: 10px;\n"
"font-weight: bold;\n"
"border-radius: 8px;\n"
"font-size: 14px;"));

        horizontalLayoutFaceButtonsp->addWidget(btnVerifyp);

        btnCancelFacep = new QPushButton(dialogFaceIDp);
        btnCancelFacep->setObjectName("btnCancelFacep");
        btnCancelFacep->setStyleSheet(QString::fromUtf8("background-color: #e74c3c;\n"
"border: 2px solid #c0392b;\n"
"color: white;\n"
"padding: 10px;\n"
"font-weight: bold;\n"
"border-radius: 8px;\n"
"font-size: 14px;"));

        horizontalLayoutFaceButtonsp->addWidget(btnCancelFacep);


        verticalLayoutFaceID->addLayout(horizontalLayoutFaceButtonsp);

        labelFaceInfop = new QLabel(dialogFaceIDp);
        labelFaceInfop->setObjectName("labelFaceInfop");
        labelFaceInfop->setStyleSheet(QString::fromUtf8("color: #3498db;\n"
"font-size: 12px;\n"
"text-align: center;\n"
"padding: 5px;\n"
"background-color: rgba(255, 255, 255, 0.1);\n"
"border-radius: 5px;"));
        labelFaceInfop->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayoutFaceID->addWidget(labelFaceInfop);

        pushButton_4p = new QPushButton(pagepecheur);
        pushButton_4p->setObjectName("pushButton_4p");
        pushButton_4p->setGeometry(QRect(1350, 230, 71, 61));
        pushButton_4p->setStyleSheet(QString::fromUtf8("border:none;\n"
"background-color:transparent;"));
        pushButton_5p = new QPushButton(pagepecheur);
        pushButton_5p->setObjectName("pushButton_5p");
        pushButton_5p->setGeometry(QRect(1390, 230, 61, 61));
        pushButton_5p->setStyleSheet(QString::fromUtf8("border:none;\n"
"background-color:transparent;"));
        pushButton_6p = new QPushButton(pagepecheur);
        pushButton_6p->setObjectName("pushButton_6p");
        pushButton_6p->setGeometry(QRect(1420, 230, 61, 61));
        pushButton_6p->setStyleSheet(QString::fromUtf8("border:none;\n"
"background-color:transparent;"));
        frame_typesp = new QFrame(pagepecheur);
        frame_typesp->setObjectName("frame_typesp");
        frame_typesp->setGeometry(QRect(660, 540, 431, 281));
        frame_typesp->setStyleSheet(QString::fromUtf8("background-color: rgba(0, 0, 50, 0.7);\n"
"            border: 2px solid #59abc8;\n"
"            border-radius: 15px;"));
        frame_typesp->setFrameShape(QFrame::Shape::StyledPanel);
        frame_typesp->setFrameShadow(QFrame::Shadow::Raised);
        verticalLayout_types = new QVBoxLayout(frame_typesp);
        verticalLayout_types->setObjectName("verticalLayout_types");
        label_typesp = new QLabel(frame_typesp);
        label_typesp->setObjectName("label_typesp");
        label_typesp->setStyleSheet(QString::fromUtf8("color: #f3ffbe;\n"
"                        font-weight: bold;\n"
"                        font-size: 17px;\n"
"                        padding: 10px;"));
        label_typesp->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout_types->addWidget(label_typesp);

        horizontalLayout_typesp = new QHBoxLayout();
        horizontalLayout_typesp->setObjectName("horizontalLayout_typesp");
        progressTypeCirclep = new QLabel(frame_typesp);
        progressTypeCirclep->setObjectName("progressTypeCirclep");
        progressTypeCirclep->setMinimumSize(QSize(200, 200));
        progressTypeCirclep->setStyleSheet(QString::fromUtf8("border-radius: 100px;\n"
"        background: qconicalgradient(cx:0.5, cy:0.5, angle:90,\n"
"            stop:0.0 #3498db,\n"
"            stop:0.25 #27ae60,\n"
"            stop:0.25 #f39c12,\n"
"            stop:0.55 #9b59b6,\n"
"            stop:0.55 #e74c3c,\n"
"            stop:0.85 #1abc9c,\n"
"            stop:0.85 rgba(26, 188, 156, 0.3) 100%);\n"
"        border: 5px solid  rgb(74, 82, 90);\n"
""));
        progressTypeCirclep->setAlignment(Qt::AlignmentFlag::AlignCenter);

        horizontalLayout_typesp->addWidget(progressTypeCirclep);

        verticalLayout_legendp = new QVBoxLayout();
        verticalLayout_legendp->setObjectName("verticalLayout_legendp");
        label_total_typesp = new QLabel(frame_typesp);
        label_total_typesp->setObjectName("label_total_typesp");
        label_total_typesp->setStyleSheet(QString::fromUtf8("color: grey;\n"
"                                        font-weight: bold;\n"
"                                        font-size: 16px;\n"
"                                        padding: 5px;"));
        label_total_typesp->setTextFormat(Qt::TextFormat::AutoText);

        verticalLayout_legendp->addWidget(label_total_typesp);

        label_legend_chalutierp = new QLabel(frame_typesp);
        label_legend_chalutierp->setObjectName("label_legend_chalutierp");
        label_legend_chalutierp->setStyleSheet(QString::fromUtf8("color: #3498db;\n"
"                                        font-weight: bold;\n"
"                                        font-size: 14px;\n"
"                                        padding: 3px;"));

        verticalLayout_legendp->addWidget(label_legend_chalutierp);

        label_legend_palangrierp = new QLabel(frame_typesp);
        label_legend_palangrierp->setObjectName("label_legend_palangrierp");
        label_legend_palangrierp->setStyleSheet(QString::fromUtf8("color: #27ae60;\n"
"                                        font-weight: bold;\n"
"                                        font-size: 14px;\n"
"                                        padding: 3px;"));

        verticalLayout_legendp->addWidget(label_legend_palangrierp);

        label_legend_caseyeurp = new QLabel(frame_typesp);
        label_legend_caseyeurp->setObjectName("label_legend_caseyeurp");
        label_legend_caseyeurp->setStyleSheet(QString::fromUtf8("color: #f39c12;\n"
"                                        font-weight: bold;\n"
"                                        font-size: 14px;\n"
"                                        padding: 3px;"));

        verticalLayout_legendp->addWidget(label_legend_caseyeurp);

        label_legend_traditionalp = new QLabel(frame_typesp);
        label_legend_traditionalp->setObjectName("label_legend_traditionalp");
        label_legend_traditionalp->setStyleSheet(QString::fromUtf8("color: #9b59b6;\n"
"                                        font-weight: bold;\n"
"                                        font-size: 14px;\n"
"                                        padding: 3px;"));

        verticalLayout_legendp->addWidget(label_legend_traditionalp);

        label_legend_otherp = new QLabel(frame_typesp);
        label_legend_otherp->setObjectName("label_legend_otherp");
        label_legend_otherp->setStyleSheet(QString::fromUtf8("color: #e74c3c;\n"
"                                        font-weight: bold;\n"
"                                        font-size: 14px;\n"
"                                        padding: 3px;"));

        verticalLayout_legendp->addWidget(label_legend_otherp);


        horizontalLayout_typesp->addLayout(verticalLayout_legendp);


        verticalLayout_types->addLayout(horizontalLayout_typesp);

        labelfp = new QLabel(pagepecheur);
        labelfp->setObjectName("labelfp");
        labelfp->setGeometry(QRect(90, 90, 141, 31));
        label_2p = new QLabel(pagepecheur);
        label_2p->setObjectName("label_2p");
        label_2p->setGeometry(QRect(620, 130, 61, 21));
        comboBox_6p = new QComboBox(pagepecheur);
        comboBox_6p->addItem(QString());
        comboBox_6p->addItem(QString());
        comboBox_6p->addItem(QString());
        comboBox_6p->addItem(QString());
        comboBox_6p->addItem(QString());
        comboBox_6p->setObjectName("comboBox_6p");
        comboBox_6p->setGeometry(QRect(980, 120, 231, 41));
        comboBox_6p->setEditable(true);
        label_4p = new QLabel(pagepecheur);
        label_4p->setObjectName("label_4p");
        label_4p->setGeometry(QRect(850, 120, 121, 31));
        brmp = new QPushButton(pagepecheur);
        brmp->setObjectName("brmp");
        brmp->setGeometry(QRect(40, 740, 141, 61));
        bep = new QPushButton(pagepecheur);
        bep->setObjectName("bep");
        bep->setGeometry(QRect(1310, 740, 181, 61));
        framefaceidp = new QFrame(pagepecheur);
        framefaceidp->setObjectName("framefaceidp");
        framefaceidp->setGeometry(QRect(510, 130, 881, 531));
        framefaceidp->setFrameShape(QFrame::Shape::StyledPanel);
        framefaceidp->setFrameShadow(QFrame::Shadow::Raised);
        faceGuideCircle_4p = new QLabel(framefaceidp);
        faceGuideCircle_4p->setObjectName("faceGuideCircle_4p");
        faceGuideCircle_4p->setGeometry(QRect(80, 140, 220, 220));
        faceGuideCircle_4p->setMinimumSize(QSize(220, 220));
        faceGuideCircle_4p->setMaximumSize(QSize(220, 220));
        faceGuideCircle_4p->setStyleSheet(QString::fromUtf8("border: 3px dashed rgba(0, 250, 150, 0.7);\n"
"border-radius: 110px;\n"
"background: transparent;"));
        label_zones_4p = new QLabel(framefaceidp);
        label_zones_4p->setObjectName("label_zones_4p");
        label_zones_4p->setGeometry(QRect(30, 20, 821, 46));
        label_zones_4p->setStyleSheet(QString::fromUtf8("color: #f3ffbe;\n"
"                    font-weight: bold;\n"
"                    font-size: 17px;\n"
"                    padding: 10px;"));
        label_zones_4p->setAlignment(Qt::AlignmentFlag::AlignCenter);
        frame_10p = new QFrame(framefaceidp);
        frame_10p->setObjectName("frame_10p");
        frame_10p->setGeometry(QRect(50, 130, 281, 241));
        frame_10p->setStyleSheet(QString::fromUtf8(""));
        frame_10p->setFrameShape(QFrame::Shape::StyledPanel);
        frame_10p->setFrameShadow(QFrame::Shadow::Raised);
        label_legend_chalutier_11p = new QLabel(framefaceidp);
        label_legend_chalutier_11p->setObjectName("label_legend_chalutier_11p");
        label_legend_chalutier_11p->setGeometry(QRect(80, 80, 251, 41));
        label_legend_chalutier_11p->setStyleSheet(QString::fromUtf8("color: white;\n"
"                                        \n"
"                                        font-size: 20px;\n"
"                                        padding: 3px;\n"
" border-radius: 15px;"));
        label_legend_chalutier_12p = new QLabel(framefaceidp);
        label_legend_chalutier_12p->setObjectName("label_legend_chalutier_12p");
        label_legend_chalutier_12p->setGeometry(QRect(50, 490, 791, 31));
        label_legend_chalutier_12p->setStyleSheet(QString::fromUtf8("color: white;\n"
"                                        \n"
"                                        font-size: 20px;\n"
"                                        padding: 3px;\n"
"border-radius: 15px;"));
        frame_11p = new QFrame(framefaceidp);
        frame_11p->setObjectName("frame_11p");
        frame_11p->setGeometry(QRect(450, 129, 391, 201));
        frame_11p->setFrameShape(QFrame::Shape::StyledPanel);
        frame_11p->setFrameShadow(QFrame::Shadow::Raised);
        label_legend_chalutier_13p = new QLabel(frame_11p);
        label_legend_chalutier_13p->setObjectName("label_legend_chalutier_13p");
        label_legend_chalutier_13p->setGeometry(QRect(10, 10, 361, 41));
        label_legend_chalutier_13p->setStyleSheet(QString::fromUtf8("color: #3498db;\n"
"                                        \n"
"                                        font-size: 20px;\n"
"                                        padding: 3px;\n"
" border-radius: 15px;"));
        lineEdit_11p = new QLineEdit(frame_11p);
        lineEdit_11p->setObjectName("lineEdit_11p");
        lineEdit_11p->setGeometry(QRect(20, 80, 341, 31));
        lineEdit_12p = new QLineEdit(frame_11p);
        lineEdit_12p->setObjectName("lineEdit_12p");
        lineEdit_12p->setGeometry(QRect(20, 140, 341, 31));
        pushButton_10p = new QPushButton(framefaceidp);
        pushButton_10p->setObjectName("pushButton_10p");
        pushButton_10p->setGeometry(QRect(520, 370, 291, 61));
        pushButton_10p->setStyleSheet(QString::fromUtf8("\n"
"background-color: rgba(0, 80, 120, 0.7);\n"
"  "));
        pushButton_11p = new QPushButton(framefaceidp);
        pushButton_11p->setObjectName("pushButton_11p");
        pushButton_11p->setGeometry(QRect(520, 430, 291, 61));
        pushButton_11p->setStyleSheet(QString::fromUtf8("background-color: rgba(80, 0, 0, 0.7);\n"
""));
        bmi_6p = new QPushButton(framefaceidp);
        bmi_6p->setObjectName("bmi_6p");
        bmi_6p->setGeometry(QRect(50, 440, 291, 61));
        bmi_6p->setStyleSheet(QString::fromUtf8("background-color:rgba(0, 80, 0, 0.7);"));
        label_8p = new QLabel(framefaceidp);
        label_8p->setObjectName("label_8p");
        label_8p->setGeometry(QRect(110, 390, 181, 61));
        label_8p->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 140, 0, 0.7);\n"
"color: white;\n"
"box-shadow: 0px 0px 15px #FFB74D;\n"
"color: white;\n"
"    font-weight: 700;\n"
"    font-size: 15px;\n"
"    margin-bottom:20px;\n"
"    text-shadow: 0 1px 2px rgba(0, 0, 0, 0.3);\n"
"padding: 10px 20px;"));
        stackedWidget->addWidget(pagepecheur);
        page5 = new QWidget();
        page5->setObjectName("page5");
        stackedWidget->addWidget(page5);
        Gpecheurs->setCentralWidget(centralwidget);
        menubar = new QMenuBar(Gpecheurs);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1512, 25));
        Gpecheurs->setMenuBar(menubar);
        statusbar = new QStatusBar(Gpecheurs);
        statusbar->setObjectName("statusbar");
        Gpecheurs->setStatusBar(statusbar);

        retranslateUi(Gpecheurs);

        QMetaObject::connectSlotsByName(Gpecheurs);
    } // setupUi

    void retranslateUi(QMainWindow *Gpecheurs)
    {
        Gpecheurs->setWindowTitle(QCoreApplication::translate("Gpecheurs", "Gpecheurs", nullptr));
        labelp->setText(QCoreApplication::translate("Gpecheurs", "Gestion Des P\303\252cheurs", nullptr));
        lineEditp->setText(QString());
        lineEditp->setPlaceholderText(QCoreApplication::translate("Gpecheurs", "ID", nullptr));
        lineEdit_2p->setText(QString());
        lineEdit_2p->setPlaceholderText(QCoreApplication::translate("Gpecheurs", "NOM", nullptr));
        comboBoxp->setItemText(0, QCoreApplication::translate("Gpecheurs", "Capitaine", nullptr));
        comboBoxp->setItemText(1, QCoreApplication::translate("Gpecheurs", "Marin", nullptr));
        comboBoxp->setItemText(2, QCoreApplication::translate("Gpecheurs", "Autre", nullptr));

        comboBoxp->setCurrentText(QCoreApplication::translate("Gpecheurs", "Capitaine", nullptr));
        comboBox_2->setItemText(0, QCoreApplication::translate("Gpecheurs", "Disponible", nullptr));
        comboBox_2->setItemText(1, QCoreApplication::translate("Gpecheurs", "Indisponible", nullptr));

        comboBox_2->setCurrentText(QCoreApplication::translate("Gpecheurs", "Disponible", nullptr));
        comboBox_3p->setItemText(0, QCoreApplication::translate("Gpecheurs", "Bateaux", nullptr));

        lineEdit_3p->setText(QString());
        lineEdit_3p->setPlaceholderText(QCoreApplication::translate("Gpecheurs", "Pr\303\251nom", nullptr));
#if QT_CONFIG(tooltip)
        btnFaceIDp->setToolTip(QCoreApplication::translate("Gpecheurs", "Capturer le visage pour reconnaissance faciale", nullptr));
#endif // QT_CONFIG(tooltip)
        btnFaceIDp->setText(QCoreApplication::translate("Gpecheurs", "\360\237\221\244 Enregistrer Visage", nullptr));
        bmip->setText(QCoreApplication::translate("Gpecheurs", "\342\234\205Mission", nullptr));
        bap->setText(QCoreApplication::translate("Gpecheurs", "Ajouter", nullptr));
        label_5p->setText(QCoreApplication::translate("Gpecheurs", "R\303\264le:", nullptr));
        label_6p->setText(QCoreApplication::translate("Gpecheurs", "Disponibilit\303\251:", nullptr));
        label_7p->setText(QCoreApplication::translate("Gpecheurs", "Bateaux:", nullptr));
        QTableWidgetItem *___qtablewidgetitem = tableWidgetp->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("Gpecheurs", "ID", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = tableWidgetp->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("Gpecheurs", "Nom", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = tableWidgetp->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("Gpecheurs", "Prenom", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = tableWidgetp->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("Gpecheurs", "R\303\264le", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = tableWidgetp->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("Gpecheurs", "Bateaux", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = tableWidgetp->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QCoreApplication::translate("Gpecheurs", "Disponibilit\303\251", nullptr));
        QTableWidgetItem *___qtablewidgetitem6 = tableWidgetp->horizontalHeaderItem(6);
        ___qtablewidgetitem6->setText(QCoreApplication::translate("Gpecheurs", "Email", nullptr));
        QTableWidgetItem *___qtablewidgetitem7 = tableWidgetp->horizontalHeaderItem(7);
        ___qtablewidgetitem7->setText(QCoreApplication::translate("Gpecheurs", "Date", nullptr));
        QTableWidgetItem *___qtablewidgetitem8 = tableWidgetp->horizontalHeaderItem(8);
        ___qtablewidgetitem8->setText(QCoreApplication::translate("Gpecheurs", "Heure", nullptr));
        QTableWidgetItem *___qtablewidgetitem9 = tableWidgetp->horizontalHeaderItem(9);
        ___qtablewidgetitem9->setText(QCoreApplication::translate("Gpecheurs", "Face Id", nullptr));
        QTableWidgetItem *___qtablewidgetitem10 = tableWidgetp->horizontalHeaderItem(10);
        ___qtablewidgetitem10->setText(QCoreApplication::translate("Gpecheurs", "Action", nullptr));
        comboBox_5p->setItemText(0, QCoreApplication::translate("Gpecheurs", "\360\237\221\245 Tous les r\303\264les", nullptr));
        comboBox_5p->setItemText(1, QCoreApplication::translate("Gpecheurs", "\360\237\221\250\342\200\215\342\234\210\357\270\217 Capitaine", nullptr));
        comboBox_5p->setItemText(2, QCoreApplication::translate("Gpecheurs", "\360\237\247\221\342\200\215\342\234\210\357\270\217  Contrema\303\256tre", nullptr));
        comboBox_5p->setItemText(3, QCoreApplication::translate("Gpecheurs", "\360\237\221\250\342\200\215\360\237\214\276Ma\303\256tre de p\303\252che", nullptr));
        comboBox_5p->setItemText(4, QCoreApplication::translate("Gpecheurs", "\360\237\221\250\342\200\215\360\237\214\276Marin-p\303\252cheur", nullptr));
        comboBox_5p->setItemText(5, QCoreApplication::translate("Gpecheurs", "\360\237\247\222apprenti p\303\252cheur", nullptr));

        comboBox_5p->setCurrentText(QCoreApplication::translate("Gpecheurs", "\360\237\221\245 Tous les r\303\264les", nullptr));
        lineEdit_4p->setText(QString());
        lineEdit_4p->setPlaceholderText(QCoreApplication::translate("Gpecheurs", "Rechercher", nullptr));
        label_3p->setText(QCoreApplication::translate("Gpecheurs", "\360\237\224\216:", nullptr));
        dialogFaceIDp->setWindowTitle(QCoreApplication::translate("Gpecheurs", "\360\237\221\244 Reconnaissance Faciale", nullptr));
        labelFaceTitlep->setText(QCoreApplication::translate("Gpecheurs", "\360\237\224\220 Reconnaissance Faciale", nullptr));
        labelCamerap->setText(QCoreApplication::translate("Gpecheurs", "\360\237\223\267 Cam\303\251ra", nullptr));
        labelFaceStatusp->setText(QCoreApplication::translate("Gpecheurs", "Pr\303\252t \303\240 capturer...", nullptr));
        btnCapturep->setText(QCoreApplication::translate("Gpecheurs", "\360\237\223\270 Capturer", nullptr));
        btnVerifyp->setText(QCoreApplication::translate("Gpecheurs", "\342\234\205 V\303\251rifier", nullptr));
        btnCancelFacep->setText(QCoreApplication::translate("Gpecheurs", "\342\235\214 Fermer", nullptr));
        labelFaceInfop->setText(QCoreApplication::translate("Gpecheurs", "P\303\252cheur : [Nom]", nullptr));
        pushButton_4p->setText(QCoreApplication::translate("Gpecheurs", "\342\234\217\357\270\217", nullptr));
        pushButton_5p->setText(QCoreApplication::translate("Gpecheurs", "\342\235\214", nullptr));
        pushButton_6p->setText(QCoreApplication::translate("Gpecheurs", "\360\237\224\204", nullptr));
        label_typesp->setText(QCoreApplication::translate("Gpecheurs", "\360\237\223\210Statistiques Selon Disponibilit\303\251", nullptr));
        progressTypeCirclep->setText(QString());
        label_total_typesp->setText(QCoreApplication::translate("Gpecheurs", "       Nom et Prenom", nullptr));
        label_legend_chalutierp->setText(QCoreApplication::translate("Gpecheurs", "\342\227\217 Disponible: 0 (00%)", nullptr));
        label_legend_palangrierp->setText(QCoreApplication::translate("Gpecheurs", "\342\227\217 Sous Condition: 0 (00%)", nullptr));
        label_legend_caseyeurp->setText(QCoreApplication::translate("Gpecheurs", "\342\227\217 Indisponible: 0 (00%)", nullptr));
        label_legend_traditionalp->setText(QCoreApplication::translate("Gpecheurs", "\342\227\217 En Cong\303\251: 0 (00%)", nullptr));
        label_legend_otherp->setText(QCoreApplication::translate("Gpecheurs", "\342\227\217 Autres: 0 (00%)", nullptr));
        labelfp->setText(QCoreApplication::translate("Gpecheurs", "    Formulaire", nullptr));
        label_2p->setText(QCoreApplication::translate("Gpecheurs", "R\303\264le:", nullptr));
        comboBox_6p->setItemText(0, QCoreApplication::translate("Gpecheurs", "\360\237\223\213 Toutes les disponibilit\303\251s", nullptr));
        comboBox_6p->setItemText(1, QCoreApplication::translate("Gpecheurs", "\342\234\205 Disponible", nullptr));
        comboBox_6p->setItemText(2, QCoreApplication::translate("Gpecheurs", "\342\217\263 Disponible bient\303\264t", nullptr));
        comboBox_6p->setItemText(3, QCoreApplication::translate("Gpecheurs", "\342\235\214 Indisponible", nullptr));
        comboBox_6p->setItemText(4, QCoreApplication::translate("Gpecheurs", "\360\237\217\235\357\270\217 En cong\303\251", nullptr));

        comboBox_6p->setCurrentText(QCoreApplication::translate("Gpecheurs", "\360\237\223\213 Toutes les disponibilit\303\251s", nullptr));
        label_4p->setText(QCoreApplication::translate("Gpecheurs", "Disponibilit\303\251:", nullptr));
        brmp->setText(QCoreApplication::translate("Gpecheurs", "Retour Menu", nullptr));
        bep->setText(QCoreApplication::translate("Gpecheurs", "\360\237\223\244Exporter en PDF", nullptr));
        faceGuideCircle_4p->setText(QString());
        label_zones_4p->setText(QCoreApplication::translate("Gpecheurs", "\360\237\224\215Enregistrement Biom\303\251trique", nullptr));
        label_legend_chalutier_11p->setText(QCoreApplication::translate("Gpecheurs", "Positionnez Votre Visage", nullptr));
        label_legend_chalutier_12p->setText(QCoreApplication::translate("Gpecheurs", "                         \360\237\222\241 Assurez-vous d'\303\252tre dans un endroit bien \303\251clair\303\251", nullptr));
        label_legend_chalutier_13p->setText(QCoreApplication::translate("Gpecheurs", "        P\303\252cheur En Enregistrement", nullptr));
        lineEdit_11p->setText(QString());
        lineEdit_11p->setPlaceholderText(QCoreApplication::translate("Gpecheurs", "Nom & Pr\303\251nom", nullptr));
        lineEdit_12p->setText(QString());
        lineEdit_12p->setPlaceholderText(QCoreApplication::translate("Gpecheurs", "R\303\264le", nullptr));
        pushButton_10p->setText(QCoreApplication::translate("Gpecheurs", "\360\237\224\204R\303\251essayer", nullptr));
        pushButton_11p->setText(QCoreApplication::translate("Gpecheurs", "\342\235\214 Annuler", nullptr));
        bmi_6p->setText(QCoreApplication::translate("Gpecheurs", "\342\234\205FaceID Valider", nullptr));
        label_8p->setText(QCoreApplication::translate("Gpecheurs", "\360\237\225\265\360\237\217\273Visage D\303\251t\303\251ct\303\251", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Gpecheurs: public Ui_Gpecheurs {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GPECHEURS_H
