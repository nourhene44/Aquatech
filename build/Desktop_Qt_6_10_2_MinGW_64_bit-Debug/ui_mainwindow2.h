/********************************************************************************
** Form generated from reading UI file 'mainwindow2.ui'
**
** Created by: Qt User Interface Compiler version 6.10.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW2_H
#define UI_MAINWINDOW2_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
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

class Ui_MainWindow2
{
public:
    QWidget *centralwidgete;
    QStackedWidget *stackedWidgete;
    QWidget *pagee;
    QFrame *frame_remplacement_iae;
    QVBoxLayout *verticalLayout;
    QLabel *lbl_titre_iae;
    QLabel *lbl_alertee;
    QLabel *lbl_desce;
    QHBoxLayout *_2e;
    QLabel *labele_2;
    QLabel *labele;
    QPushButton *btn_affecter_1e;
    QHBoxLayout *_3e;
    QLabel *labele_3e;
    QLabel *labelee;
    QPushButton *btn_affecter_2e;
    QHBoxLayout *_4;
    QLabel *labeleee;
    QLabel *labeleeee;
    QPushButton *btn_affecter_3e;
    QPushButton *btn_voir_remplacantse;
    QPushButton *pushButton_6e;
    QFrame *frame_2e;
    QLineEdit *lineEdit_12e;
    QLineEdit *lineEdit_13e;
    QPushButton *pushButton_5e;
    QLineEdit *lineEdit_16e;
    QLineEdit *lineEdit_14e;
    QComboBox *comboBox_11e;
    QComboBox *comboBox_14e;
    QComboBox *comboBox_15e;
    QLabel *label_3e;
    QLabel *label_5e;
    QLabel *label_6e;
    QLabel *label_2e;
    QComboBox *comboBox_16e;
    QLineEdit *lineEdit_15e;
    QPushButton *pushButton_7e;
    QPushButton *pushButton_8e;
    QFrame *frame_typese;
    QVBoxLayout *verticalLayout_types;
    QLabel *label_typese;
    QHBoxLayout *horizontalLayout_typese;
    QLabel *progressTypeCirclee;
    QVBoxLayout *verticalLayout_legende;
    QLabel *label_total_typese;
    QLabel *label_legend_chalutiere;
    QLabel *label_legend_palangriere;
    QLabel *label_legend_caseyeure;
    QLabel *label_legend_traditionale;
    QLabel *label_legend_othere;
    QLabel *label_8e;
    QPushButton *pushButton_10e;
    QPushButton *pushButton_11e;
    QLabel *label_7e;
    QLabel *label_9e;
    QComboBox *comboBox_12e;
    QLabel *label_10e;
    QComboBox *comboBox_17e;
    QLineEdit *lineEdit_17e;
    QTableWidget *tableWidgetee;
    QPushButton *pushButton_9e;
    QWidget *page_2;
    QLabel *label_4e;
    QMenuBar *menubare;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow2)
    {
        if (MainWindow2->objectName().isEmpty())
            MainWindow2->setObjectName("MainWindow2");
        MainWindow2->resize(1333, 849);
        MainWindow2->setStyleSheet(QString::fromUtf8(""));
        centralwidgete = new QWidget(MainWindow2);
        centralwidgete->setObjectName("centralwidgete");
        stackedWidgete = new QStackedWidget(centralwidgete);
        stackedWidgete->setObjectName("stackedWidgete");
        stackedWidgete->setGeometry(QRect(0, -10, 1331, 801));
        stackedWidgete->setStyleSheet(QString::fromUtf8("#pagee {\n"
"	background-image: url(:/new/prefix2/bg.png);\n"
"    background-repeat: no-repeat;\n"
"    background-position: center;\n"
"    background-size: cover;\n"
"border-radius:15px;\n"
"}\n"
"\n"
"\n"
"\n"
"QComboBox{\n"
"    font-size: 16px;\n"
"    padding: 6px;\n"
"    border: 2px solid #59abc8;\n"
"    border-radius: 15px;\n"
"	background-color: rgba(0, 0, 50, 0.8); \n"
"}\n"
"\n"
"#label_3e,#label_4e,#label_5e,#label_6e,#labeel,#label_9e,#label_10e,#label_2e\n"
"{border:none;\n"
"background-color:transparent;\n"
"font-size: 17px;   \n"
"    padding: 1px;\n"
"}\n"
"#label_7e,#label_8e{\n"
"    color:#fffae0;\n"
"    font-weight: 600;\n"
"    font-size: 17px;\n"
"    padding: 5px 0;\n"
"}\n"
"* {\n"
"    font-family: \"Segoe UI\", \"Century Gothic\", sans-serif;\n"
"    font-size: 14px;\n"
"    font-weight: 400;\n"
"}\n"
"QFrame {\n"
"     border-radius: 15px;\n"
" 		background-color: rgba(0, 0, 50, 0.7); \n"
"\n"
"\n"
"    border: 2px solid #59abc8 ; \n"
"}\n"
"QPushButton {\n"
"    border-radius: "
                        "15px;\n"
"background-color: rgba(0, 0, 150, 0.7); \n"
" color: white;\n"
"   border: 2px solid #59abc8;  /* \330\245\330\267\330\247\330\261 \330\243\330\262\330\261\331\202 \330\272\330\247\331\205\331\202 */\n"
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
"\n"
"QLineEdit{\n"
"font-size: 17px;\n"
"    background: transparent;  \n"
"    border: none;\n"
"    color: white;\n"
"    border-bottom: 2px solid #59abc8; \n"
"    padding: 3px;\n"
" selection-background-color: white; \n"
"   \n"
"}\n"
"QLineEdit:focus {\n"
"\n"
"    color: #59abc8;\n"
"}\n"
""));
        pagee = new QWidget();
        pagee->setObjectName("pagee");
        frame_remplacement_iae = new QFrame(pagee);
        frame_remplacement_iae->setObjectName("frame_remplacement_iae");
        frame_remplacement_iae->setGeometry(QRect(300, 400, 541, 380));
        frame_remplacement_iae->setMinimumSize(QSize(420, 380));
        frame_remplacement_iae->setStyleSheet(QString::fromUtf8("\n"
"    QFrame#frame_remplacement_ia {\n"
"        background-color: rgba(10, 20, 60, 170);\n"
"        border: 2px solid #3daee9;\n"
"        border-radius: 18px;\n"
"    }\n"
"    QLabel { color: white; }\n"
"    QPushButton {\n"
"        background-color: #1e90ff;\n"
"        color: white;\n"
"        border-radius: 10px;\n"
"        padding: 6px 12px;\n"
"    }\n"
"    QPushButton:hover {\n"
"        background-color: #3daee9;\n"
"    }\n"
"      "));
        verticalLayout = new QVBoxLayout(frame_remplacement_iae);
        verticalLayout->setObjectName("verticalLayout");
        lbl_titre_iae = new QLabel(frame_remplacement_iae);
        lbl_titre_iae->setObjectName("lbl_titre_iae");
        lbl_titre_iae->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout->addWidget(lbl_titre_iae);

        lbl_alertee = new QLabel(frame_remplacement_iae);
        lbl_alertee->setObjectName("lbl_alertee");

        verticalLayout->addWidget(lbl_alertee);

        lbl_desce = new QLabel(frame_remplacement_iae);
        lbl_desce->setObjectName("lbl_desce");
        lbl_desce->setWordWrap(true);

        verticalLayout->addWidget(lbl_desce);

        _2e = new QHBoxLayout();
        _2e->setObjectName("_2e");
        labele_2 = new QLabel(frame_remplacement_iae);
        labele_2->setObjectName("labele_2");

        _2e->addWidget(labele_2);

        labele = new QLabel(frame_remplacement_iae);
        labele->setObjectName("labele");
        labele->setAlignment(Qt::AlignmentFlag::AlignCenter);

        _2e->addWidget(labele);

        btn_affecter_1e = new QPushButton(frame_remplacement_iae);
        btn_affecter_1e->setObjectName("btn_affecter_1e");

        _2e->addWidget(btn_affecter_1e);


        verticalLayout->addLayout(_2e);

        _3e = new QHBoxLayout();
        _3e->setObjectName("_3e");
        labele_3e = new QLabel(frame_remplacement_iae);
        labele_3e->setObjectName("labele_3e");

        _3e->addWidget(labele_3e);

        labelee = new QLabel(frame_remplacement_iae);
        labelee->setObjectName("labelee");
        labelee->setAlignment(Qt::AlignmentFlag::AlignCenter);

        _3e->addWidget(labelee);

        btn_affecter_2e = new QPushButton(frame_remplacement_iae);
        btn_affecter_2e->setObjectName("btn_affecter_2e");

        _3e->addWidget(btn_affecter_2e);


        verticalLayout->addLayout(_3e);

        _4 = new QHBoxLayout();
        _4->setObjectName("_4");
        labeleee = new QLabel(frame_remplacement_iae);
        labeleee->setObjectName("labeleee");

        _4->addWidget(labeleee);

        labeleeee = new QLabel(frame_remplacement_iae);
        labeleeee->setObjectName("labeleeee");
        labeleeee->setAlignment(Qt::AlignmentFlag::AlignCenter);

        _4->addWidget(labeleeee);

        btn_affecter_3e = new QPushButton(frame_remplacement_iae);
        btn_affecter_3e->setObjectName("btn_affecter_3e");

        _4->addWidget(btn_affecter_3e);


        verticalLayout->addLayout(_4);

        btn_voir_remplacantse = new QPushButton(frame_remplacement_iae);
        btn_voir_remplacantse->setObjectName("btn_voir_remplacantse");

        verticalLayout->addWidget(btn_voir_remplacantse);

        pushButton_6e = new QPushButton(pagee);
        pushButton_6e->setObjectName("pushButton_6e");
        pushButton_6e->setGeometry(QRect(1230, 280, 71, 61));
        frame_2e = new QFrame(pagee);
        frame_2e->setObjectName("frame_2e");
        frame_2e->setGeometry(QRect(10, 110, 271, 511));
        frame_2e->setFrameShape(QFrame::Shape::StyledPanel);
        frame_2e->setFrameShadow(QFrame::Shadow::Raised);
        lineEdit_12e = new QLineEdit(frame_2e);
        lineEdit_12e->setObjectName("lineEdit_12e");
        lineEdit_12e->setGeometry(QRect(20, 50, 231, 28));
        lineEdit_13e = new QLineEdit(frame_2e);
        lineEdit_13e->setObjectName("lineEdit_13e");
        lineEdit_13e->setGeometry(QRect(20, 100, 231, 28));
        pushButton_5e = new QPushButton(frame_2e);
        pushButton_5e->setObjectName("pushButton_5e");
        pushButton_5e->setGeometry(QRect(80, 460, 101, 61));
        lineEdit_16e = new QLineEdit(frame_2e);
        lineEdit_16e->setObjectName("lineEdit_16e");
        lineEdit_16e->setGeometry(QRect(20, 140, 231, 28));
        lineEdit_14e = new QLineEdit(frame_2e);
        lineEdit_14e->setObjectName("lineEdit_14e");
        lineEdit_14e->setGeometry(QRect(20, 420, 231, 28));
        lineEdit_14e->setReadOnly(false);
        comboBox_11e = new QComboBox(frame_2e);
        comboBox_11e->addItem(QString());
        comboBox_11e->addItem(QString());
        comboBox_11e->addItem(QString());
        comboBox_11e->addItem(QString());
        comboBox_11e->addItem(QString());
        comboBox_11e->setObjectName("comboBox_11e");
        comboBox_11e->setEnabled(true);
        comboBox_11e->setGeometry(QRect(120, 180, 141, 41));
        comboBox_11e->setEditable(true);
        comboBox_11e->setDuplicatesEnabled(false);
        comboBox_14e = new QComboBox(frame_2e);
        comboBox_14e->addItem(QString());
        comboBox_14e->addItem(QString());
        comboBox_14e->addItem(QString());
        comboBox_14e->setObjectName("comboBox_14e");
        comboBox_14e->setEnabled(true);
        comboBox_14e->setGeometry(QRect(120, 230, 141, 41));
        comboBox_14e->setEditable(true);
        comboBox_15e = new QComboBox(frame_2e);
        comboBox_15e->addItem(QString());
        comboBox_15e->addItem(QString());
        comboBox_15e->addItem(QString());
        comboBox_15e->addItem(QString());
        comboBox_15e->addItem(QString());
        comboBox_15e->addItem(QString());
        comboBox_15e->setObjectName("comboBox_15e");
        comboBox_15e->setEnabled(true);
        comboBox_15e->setGeometry(QRect(120, 290, 141, 41));
        comboBox_15e->setEditable(true);
        label_3e = new QLabel(frame_2e);
        label_3e->setObjectName("label_3e");
        label_3e->setGeometry(QRect(20, 290, 91, 41));
        label_5e = new QLabel(frame_2e);
        label_5e->setObjectName("label_5e");
        label_5e->setGeometry(QRect(20, 230, 91, 41));
        label_6e = new QLabel(frame_2e);
        label_6e->setObjectName("label_6e");
        label_6e->setGeometry(QRect(20, 180, 91, 41));
        label_2e = new QLabel(frame_2e);
        label_2e->setObjectName("label_2e");
        label_2e->setGeometry(QRect(20, 340, 91, 51));
        comboBox_16e = new QComboBox(frame_2e);
        comboBox_16e->addItem(QString());
        comboBox_16e->addItem(QString());
        comboBox_16e->setObjectName("comboBox_16e");
        comboBox_16e->setEnabled(true);
        comboBox_16e->setGeometry(QRect(120, 350, 141, 41));
        comboBox_16e->setEditable(true);
        lineEdit_15e = new QLineEdit(pagee);
        lineEdit_15e->setObjectName("lineEdit_15e");
        lineEdit_15e->setGeometry(QRect(360, 180, 131, 28));
        pushButton_7e = new QPushButton(pagee);
        pushButton_7e->setObjectName("pushButton_7e");
        pushButton_7e->setGeometry(QRect(1230, 220, 61, 61));
        pushButton_8e = new QPushButton(pagee);
        pushButton_8e->setObjectName("pushButton_8e");
        pushButton_8e->setGeometry(QRect(1230, 170, 71, 61));
        frame_typese = new QFrame(pagee);
        frame_typese->setObjectName("frame_typese");
        frame_typese->setGeometry(QRect(860, 410, 441, 281));
        frame_typese->setStyleSheet(QString::fromUtf8("background-color: rgba(0, 0, 50, 0.7);\n"
"            border: 2px solid #59abc8;\n"
"            border-radius: 15px;"));
        frame_typese->setFrameShape(QFrame::Shape::StyledPanel);
        frame_typese->setFrameShadow(QFrame::Shadow::Raised);
        verticalLayout_types = new QVBoxLayout(frame_typese);
        verticalLayout_types->setObjectName("verticalLayout_types");
        label_typese = new QLabel(frame_typese);
        label_typese->setObjectName("label_typese");
        label_typese->setStyleSheet(QString::fromUtf8("color: #f3ffbe;\n"
"                        font-weight: bold;\n"
"                        font-size: 17px;\n"
"                        padding: 10px;"));
        label_typese->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout_types->addWidget(label_typese);

        horizontalLayout_typese = new QHBoxLayout();
        horizontalLayout_typese->setObjectName("horizontalLayout_typese");
        progressTypeCirclee = new QLabel(frame_typese);
        progressTypeCirclee->setObjectName("progressTypeCirclee");
        progressTypeCirclee->setMinimumSize(QSize(200, 200));
        progressTypeCirclee->setStyleSheet(QString::fromUtf8("border-radius: 100px;\n"
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
        progressTypeCirclee->setAlignment(Qt::AlignmentFlag::AlignCenter);

        horizontalLayout_typese->addWidget(progressTypeCirclee);

        verticalLayout_legende = new QVBoxLayout();
        verticalLayout_legende->setObjectName("verticalLayout_legende");
        label_total_typese = new QLabel(frame_typese);
        label_total_typese->setObjectName("label_total_typese");
        label_total_typese->setStyleSheet(QString::fromUtf8("color: white;\n"
"                                        font-weight: bold;\n"
"                                        font-size: 16px;\n"
"                                        padding: 5px;"));

        verticalLayout_legende->addWidget(label_total_typese);

        label_legend_chalutiere = new QLabel(frame_typese);
        label_legend_chalutiere->setObjectName("label_legend_chalutiere");
        label_legend_chalutiere->setStyleSheet(QString::fromUtf8("color: #3498db;\n"
"                                        font-weight: bold;\n"
"                                        font-size: 14px;\n"
"                                        padding: 3px;"));

        verticalLayout_legende->addWidget(label_legend_chalutiere);

        label_legend_palangriere = new QLabel(frame_typese);
        label_legend_palangriere->setObjectName("label_legend_palangriere");
        label_legend_palangriere->setStyleSheet(QString::fromUtf8("color: #27ae60;\n"
"                                        font-weight: bold;\n"
"                                        font-size: 14px;\n"
"                                        padding: 3px;"));

        verticalLayout_legende->addWidget(label_legend_palangriere);

        label_legend_caseyeure = new QLabel(frame_typese);
        label_legend_caseyeure->setObjectName("label_legend_caseyeure");
        label_legend_caseyeure->setStyleSheet(QString::fromUtf8("color: #f39c12;\n"
"                                        font-weight: bold;\n"
"                                        font-size: 14px;\n"
"                                        padding: 3px;"));

        verticalLayout_legende->addWidget(label_legend_caseyeure);

        label_legend_traditionale = new QLabel(frame_typese);
        label_legend_traditionale->setObjectName("label_legend_traditionale");
        label_legend_traditionale->setStyleSheet(QString::fromUtf8("color: #9b59b6;\n"
"                                        font-weight: bold;\n"
"                                        font-size: 14px;\n"
"                                        padding: 3px;"));

        verticalLayout_legende->addWidget(label_legend_traditionale);

        label_legend_othere = new QLabel(frame_typese);
        label_legend_othere->setObjectName("label_legend_othere");
        label_legend_othere->setStyleSheet(QString::fromUtf8("color: #e74c3c;\n"
"                                        font-weight: bold;\n"
"                                        font-size: 14px;\n"
"                                        padding: 3px;"));

        verticalLayout_legende->addWidget(label_legend_othere);


        horizontalLayout_typese->addLayout(verticalLayout_legende);


        verticalLayout_types->addLayout(horizontalLayout_typese);

        label_8e = new QLabel(pagee);
        label_8e->setObjectName("label_8e");
        label_8e->setGeometry(QRect(470, 20, 621, 61));
        label_8e->setStyleSheet(QString::fromUtf8("#label_8e{/* Labels sp\303\251ciaux */\n"
"    border:none;\n"
"    background-color: transparent;\n"
"    font-weight: 700;\n"
"color: rgb(66, 157, 255);\n"
"font: 25pt \"Imprint MT Shadow\";\n"
"}"));
        pushButton_10e = new QPushButton(pagee);
        pushButton_10e->setObjectName("pushButton_10e");
        pushButton_10e->setGeometry(QRect(20, 740, 151, 61));
        pushButton_11e = new QPushButton(pagee);
        pushButton_11e->setObjectName("pushButton_11e");
        pushButton_11e->setGeometry(QRect(1130, 740, 181, 61));
        label_7e = new QLabel(pagee);
        label_7e->setObjectName("label_7e");
        label_7e->setGeometry(QRect(80, 70, 101, 31));
        label_9e = new QLabel(pagee);
        label_9e->setObjectName("label_9e");
        label_9e->setGeometry(QRect(540, 80, 101, 61));
        label_9e->setStyleSheet(QString::fromUtf8("color:rgb(0, 0, 0)"));
        comboBox_12e = new QComboBox(pagee);
        comboBox_12e->addItem(QString());
        comboBox_12e->addItem(QString());
        comboBox_12e->addItem(QString());
        comboBox_12e->addItem(QString());
        comboBox_12e->addItem(QString());
        comboBox_12e->setObjectName("comboBox_12e");
        comboBox_12e->setEnabled(true);
        comboBox_12e->setGeometry(QRect(620, 90, 161, 41));
        comboBox_12e->setEditable(true);
        comboBox_12e->setDuplicatesEnabled(false);
        label_10e = new QLabel(pagee);
        label_10e->setObjectName("label_10e");
        label_10e->setGeometry(QRect(790, 80, 91, 61));
        label_10e->setStyleSheet(QString::fromUtf8("color:rgb(0, 0, 0)"));
        comboBox_17e = new QComboBox(pagee);
        comboBox_17e->addItem(QString());
        comboBox_17e->addItem(QString());
        comboBox_17e->addItem(QString());
        comboBox_17e->setObjectName("comboBox_17e");
        comboBox_17e->setEnabled(true);
        comboBox_17e->setGeometry(QRect(850, 90, 141, 41));
        comboBox_17e->setEditable(true);
        lineEdit_17e = new QLineEdit(pagee);
        lineEdit_17e->setObjectName("lineEdit_17e");
        lineEdit_17e->setGeometry(QRect(350, 97, 131, 31));
        lineEdit_17e->setStyleSheet(QString::fromUtf8("color:rgb(0, 0, 0)"));
        tableWidgetee = new QTableWidget(pagee);
        if (tableWidgetee->columnCount() < 9)
            tableWidgetee->setColumnCount(9);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tableWidgetee->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tableWidgetee->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tableWidgetee->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tableWidgetee->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        tableWidgetee->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        tableWidgetee->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        tableWidgetee->setHorizontalHeaderItem(6, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        tableWidgetee->setHorizontalHeaderItem(7, __qtablewidgetitem7);
        QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
        tableWidgetee->setHorizontalHeaderItem(8, __qtablewidgetitem8);
        tableWidgetee->setObjectName("tableWidgetee");
        tableWidgetee->setGeometry(QRect(300, 150, 911, 241));
        tableWidgetee->setStyleSheet(QString::fromUtf8("/* TableWidget */\n"
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
"}"));
        tableWidgetee->horizontalHeader()->setDefaultSectionSize(100);
        pushButton_9e = new QPushButton(pagee);
        pushButton_9e->setObjectName("pushButton_9e");
        pushButton_9e->setGeometry(QRect(1220, 20, 71, 61));
        stackedWidgete->addWidget(pagee);
        page_2 = new QWidget();
        page_2->setObjectName("page_2");
        stackedWidgete->addWidget(page_2);
        label_4e = new QLabel(centralwidgete);
        label_4e->setObjectName("label_4e");
        label_4e->setGeometry(QRect(460, 760, 91, 41));
        MainWindow2->setCentralWidget(centralwidgete);
        menubare = new QMenuBar(MainWindow2);
        menubare->setObjectName("menubare");
        menubare->setGeometry(QRect(0, 0, 1333, 25));
        MainWindow2->setMenuBar(menubare);
        statusbar = new QStatusBar(MainWindow2);
        statusbar->setObjectName("statusbar");
        MainWindow2->setStatusBar(statusbar);

        retranslateUi(MainWindow2);

        QMetaObject::connectSlotsByName(MainWindow2);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow2)
    {
        MainWindow2->setWindowTitle(QCoreApplication::translate("MainWindow2", "MainWindow2", nullptr));
        lbl_titre_iae->setStyleSheet(QCoreApplication::translate("MainWindow2", "font-size:16px;font-weight:bold;", nullptr));
        lbl_titre_iae->setText(QCoreApplication::translate("MainWindow2", "Remplacement automatique intelligent des employ\303\251s  (IA light)", nullptr));
        lbl_alertee->setStyleSheet(QCoreApplication::translate("MainWindow2", "color:#f39c12;font-weight:bold;", nullptr));
        lbl_alertee->setText(QCoreApplication::translate("MainWindow2", "\342\232\240 Employ\303\251 indisponible d\303\251tect\303\251 (Quai 2)", nullptr));
        lbl_desce->setText(QCoreApplication::translate("MainWindow2", "Proposition d\342\200\231un rempla\303\247ant bas\303\251 sur m\303\252me r\303\264le, zone et disponibilit\303\251.", nullptr));
        labele_2->setText(QCoreApplication::translate("MainWindow2", "Antoine Dubois\\nTechnicien \342\255\220\342\255\220\342\255\220\342\255\220\342\255\220", nullptr));
        labele->setStyleSheet(QCoreApplication::translate("MainWindow2", "\n"
"    background-color:#2ecc71;\n"
"    border-radius:20px;\n"
"    min-width:40px;\n"
"    font-weight:bold;\n"
"           ", nullptr));
        labele->setText(QCoreApplication::translate("MainWindow2", "87", nullptr));
        btn_affecter_1e->setText(QCoreApplication::translate("MainWindow2", "Affecter", nullptr));
        labele_3e->setText(QCoreApplication::translate("MainWindow2", "Sophie Lefevre\\nTechnicien \342\255\220\342\255\220\342\255\220\342\255\220\342\255\220", nullptr));
        labelee->setStyleSheet(QCoreApplication::translate("MainWindow2", "\n"
"    background-color:#f1c40f;\n"
"    border-radius:20px;\n"
"    min-width:40px;\n"
"    font-weight:bold;\n"
"           ", nullptr));
        labelee->setText(QCoreApplication::translate("MainWindow2", "79", nullptr));
        btn_affecter_2e->setText(QCoreApplication::translate("MainWindow2", "Affecter", nullptr));
        labeleee->setText(QCoreApplication::translate("MainWindow2", "Marc Robert\\nTechnicien \342\255\220\342\255\220\342\255\220\342\255\220", nullptr));
        labeleeee->setStyleSheet(QCoreApplication::translate("MainWindow2", "\n"
"    background-color:#e67e22;\n"
"    border-radius:20px;\n"
"    min-width:40px;\n"
"    font-weight:bold;\n"
"           ", nullptr));
        labeleeee->setText(QCoreApplication::translate("MainWindow2", "74", nullptr));
        btn_affecter_3e->setText(QCoreApplication::translate("MainWindow2", "Affecter", nullptr));
        btn_voir_remplacantse->setText(QCoreApplication::translate("MainWindow2", "Voir rempla\303\247ants", nullptr));
        pushButton_6e->setText(QCoreApplication::translate("MainWindow2", "\360\237\224\204 ", nullptr));
        lineEdit_12e->setPlaceholderText(QCoreApplication::translate("MainWindow2", "ID Employ\303\251", nullptr));
        lineEdit_13e->setPlaceholderText(QCoreApplication::translate("MainWindow2", "Nom", nullptr));
        pushButton_5e->setText(QCoreApplication::translate("MainWindow2", "Ajouter", nullptr));
        lineEdit_16e->setPlaceholderText(QCoreApplication::translate("MainWindow2", "Prenom", nullptr));
        lineEdit_14e->setPlaceholderText(QCoreApplication::translate("MainWindow2", "T\303\251l\303\251phone", nullptr));
        comboBox_11e->setItemText(0, QCoreApplication::translate("MainWindow2", "Gardien", nullptr));
        comboBox_11e->setItemText(1, QCoreApplication::translate("MainWindow2", "Technicien", nullptr));
        comboBox_11e->setItemText(2, QCoreApplication::translate("MainWindow2", "Responsable", nullptr));
        comboBox_11e->setItemText(3, QCoreApplication::translate("MainWindow2", "Ouvrier", nullptr));
        comboBox_11e->setItemText(4, QCoreApplication::translate("MainWindow2", "Pecheur", nullptr));

        comboBox_11e->setCurrentText(QCoreApplication::translate("MainWindow2", "Gardien", nullptr));
        comboBox_11e->setPlaceholderText(QString());
        comboBox_14e->setItemText(0, QCoreApplication::translate("MainWindow2", "En Mission", nullptr));
        comboBox_14e->setItemText(1, QCoreApplication::translate("MainWindow2", "Cong\303\251", nullptr));
        comboBox_14e->setItemText(2, QCoreApplication::translate("MainWindow2", "Disponible", nullptr));

        comboBox_14e->setCurrentText(QCoreApplication::translate("MainWindow2", "En Mission", nullptr));
        comboBox_15e->setItemText(0, QCoreApplication::translate("MainWindow2", "Quai 1", nullptr));
        comboBox_15e->setItemText(1, QCoreApplication::translate("MainWindow2", "Quai 2 ", nullptr));
        comboBox_15e->setItemText(2, QCoreApplication::translate("MainWindow2", "Quai 3", nullptr));
        comboBox_15e->setItemText(3, QCoreApplication::translate("MainWindow2", "Quai 4", nullptr));
        comboBox_15e->setItemText(4, QCoreApplication::translate("MainWindow2", "Stock ", nullptr));
        comboBox_15e->setItemText(5, QCoreApplication::translate("MainWindow2", "Bureau", nullptr));

        comboBox_15e->setCurrentText(QCoreApplication::translate("MainWindow2", "Quai 1", nullptr));
        label_3e->setText(QCoreApplication::translate("MainWindow2", "Zone", nullptr));
        label_5e->setText(QCoreApplication::translate("MainWindow2", "Statut", nullptr));
        label_6e->setText(QCoreApplication::translate("MainWindow2", "R\303\264les", nullptr));
        label_2e->setText(QCoreApplication::translate("MainWindow2", "Etat", nullptr));
        comboBox_16e->setItemText(0, QCoreApplication::translate("MainWindow2", "\360\237\237\242 Actif", nullptr));
        comboBox_16e->setItemText(1, QCoreApplication::translate("MainWindow2", "\360\237\224\264 Banni", nullptr));

        comboBox_16e->setCurrentText(QCoreApplication::translate("MainWindow2", "\360\237\237\242 Actif", nullptr));
        lineEdit_15e->setPlaceholderText(QCoreApplication::translate("MainWindow2", "Recherche", nullptr));
        pushButton_7e->setText(QCoreApplication::translate("MainWindow2", "\360\237\227\221\357\270\217 ", nullptr));
        pushButton_8e->setText(QCoreApplication::translate("MainWindow2", "\342\234\217\357\270\217 ", nullptr));
        label_typese->setText(QCoreApplication::translate("MainWindow2", "Statistiques par zone", nullptr));
        progressTypeCirclee->setText(QString());
        label_total_typese->setText(QCoreApplication::translate("MainWindow2", "Total: 0 employees", nullptr));
        label_legend_chalutiere->setText(QCoreApplication::translate("MainWindow2", "\342\227\217 Quai 1: 0 (00%)", nullptr));
        label_legend_palangriere->setText(QCoreApplication::translate("MainWindow2", "\342\227\217 Quai 2: 0 (00%)", nullptr));
        label_legend_caseyeure->setText(QCoreApplication::translate("MainWindow2", "\342\227\217 Stock: 0 (00%)", nullptr));
        label_legend_traditionale->setText(QCoreApplication::translate("MainWindow2", "\342\227\217 Bureau: 0 (00%)", nullptr));
        label_legend_othere->setText(QCoreApplication::translate("MainWindow2", "\342\227\217 Autres: 0 (00%)", nullptr));
        label_8e->setText(QCoreApplication::translate("MainWindow2", "Gestion des employes", nullptr));
        pushButton_10e->setText(QCoreApplication::translate("MainWindow2", "Retour Menu", nullptr));
        pushButton_11e->setText(QCoreApplication::translate("MainWindow2", "\360\237\223\204 Exporter PDF", nullptr));
        label_7e->setText(QCoreApplication::translate("MainWindow2", "Formulaire", nullptr));
        label_9e->setText(QCoreApplication::translate("MainWindow2", "R\303\264les:", nullptr));
        comboBox_12e->setItemText(0, QCoreApplication::translate("MainWindow2", "Gardien", nullptr));
        comboBox_12e->setItemText(1, QCoreApplication::translate("MainWindow2", "Technicien", nullptr));
        comboBox_12e->setItemText(2, QCoreApplication::translate("MainWindow2", "Responsable", nullptr));
        comboBox_12e->setItemText(3, QCoreApplication::translate("MainWindow2", "Ouvrier", nullptr));
        comboBox_12e->setItemText(4, QCoreApplication::translate("MainWindow2", "Pecheur", nullptr));

        comboBox_12e->setCurrentText(QCoreApplication::translate("MainWindow2", "Gardien", nullptr));
        comboBox_12e->setPlaceholderText(QString());
        label_10e->setText(QCoreApplication::translate("MainWindow2", "Statut:", nullptr));
        comboBox_17e->setItemText(0, QCoreApplication::translate("MainWindow2", "En Mission", nullptr));
        comboBox_17e->setItemText(1, QCoreApplication::translate("MainWindow2", "Cong\303\251", nullptr));
        comboBox_17e->setItemText(2, QCoreApplication::translate("MainWindow2", "Disponible", nullptr));

        comboBox_17e->setCurrentText(QCoreApplication::translate("MainWindow2", "En Mission", nullptr));
        lineEdit_17e->setPlaceholderText(QCoreApplication::translate("MainWindow2", "Recherche", nullptr));
        QTableWidgetItem *___qtablewidgetitem = tableWidgetee->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("MainWindow2", "Salaire", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = tableWidgetee->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("MainWindow2", "Etat", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = tableWidgetee->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("MainWindow2", "ID", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = tableWidgetee->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("MainWindow2", "Nom", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = tableWidgetee->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("MainWindow2", "Prenom", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = tableWidgetee->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QCoreApplication::translate("MainWindow2", "Equipe", nullptr));
        QTableWidgetItem *___qtablewidgetitem6 = tableWidgetee->horizontalHeaderItem(6);
        ___qtablewidgetitem6->setText(QCoreApplication::translate("MainWindow2", "Role", nullptr));
        QTableWidgetItem *___qtablewidgetitem7 = tableWidgetee->horizontalHeaderItem(7);
        ___qtablewidgetitem7->setText(QCoreApplication::translate("MainWindow2", "Zone", nullptr));
        QTableWidgetItem *___qtablewidgetitem8 = tableWidgetee->horizontalHeaderItem(8);
        ___qtablewidgetitem8->setText(QCoreApplication::translate("MainWindow2", "Telephone", nullptr));
        pushButton_9e->setText(QCoreApplication::translate("MainWindow2", "\360\237\214\227", nullptr));
        label_4e->setText(QCoreApplication::translate("MainWindow2", "Zone", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow2: public Ui_MainWindow2 {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW2_H
