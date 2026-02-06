/********************************************************************************
** Form generated from reading UI file 'Gclients.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GCLIENTS_H
#define UI_GCLIENTS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidgetc;
    QStackedWidget *stackedWidgetc;
    QWidget *pagec;
    QLabel *label_6c;
    QFrame *frame_2c;
    QLineEdit *lineEdit_3c;
    QLineEdit *lineEdit_4c;
    QComboBox *comboBox_2c;
    QLabel *label_14c;
    QLineEdit *lineEdit_12c;
    QLabel *label_9c;
    QComboBox *comboBoxc;
    QDateEdit *dateEdit_c;
    QLabel *label_18c;
    QPushButton *pushButton_2c;
    QLineEdit *lineEdit_14c;
    QTableWidget *tableWidgetc;
    QLabel *label_3c;
    QLineEdit *lineEdit_7c;
    QLabel *label_12c;
    QPushButton *pushButton_5c;
    QFrame *frame_c;
    QVBoxLayout *verticalLayout_zones;
    QLabel *label_zonesc;
    QHBoxLayout *horizontalLayout_c;
    QVBoxLayout *Layou_1c;
    QLabel *label_Cc;
    QProgressBar *progressCc;
    QLabel *value_Cc;
    QVBoxLayout *Layout_CCc;
    QLabel *label_CCc;
    QProgressBar *progressCCc;
    QLabel *value_CCc;
    QVBoxLayout *Layoutc;
    QLabel *label_c;
    QProgressBar *progressc;
    QLabel *value_c;
    QPushButton *pushButton_7c;
    QPushButton *pushButton_8c;
    QLabel *label_5c;
    QDateEdit *dateEdit_2c;
    QPushButton *pushButton_6c;
    QPushButton *pushButton_9c;
    QLabel *label_11c;
    QComboBox *comboBoxc_2;
    QPushButton *btnai;
    QFrame *frame_2c_2;
    QTextEdit *textChatHistory;
    QFrame *frame_2c_3;
    QLineEdit *lineEdit_3c_2;
    QPushButton *btnSend_2;
    QPushButton *btnSend;
    QLabel *label_6c_2;
    QTableWidget *tableWidget;
    QPushButton *pushButton_8c_2;
    QWidget *page_2;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1573, 885);
        centralwidgetc = new QWidget(MainWindow);
        centralwidgetc->setObjectName("centralwidgetc");
        stackedWidgetc = new QStackedWidget(centralwidgetc);
        stackedWidgetc->setObjectName("stackedWidgetc");
        stackedWidgetc->setGeometry(QRect(10, 0, 1541, 831));
        stackedWidgetc->setStyleSheet(QString::fromUtf8("#label_5c{/* Labels sp\303\251ciaux */\n"
"    border:none;\n"
"    background-color: transparent;\n"
"    font-weight: 700;\n"
"color: rgb(66, 157, 255);\n"
"font: 25pt \"Imprint MT Shadow\";\n"
"}\n"
"QPushButton {\n"
"    border-radius: 20px;\n"
"background-color: rgba(0, 0, 150, 0.7); \n"
"   border: 2px solid #59abc8; \n"
" color: white;\n"
"    font-weight: 700;\n"
"    font-size: 15px;\n"
"    margin-bottom:20px;\n"
"    text-shadow: 0 1px 2px rgba(0, 0, 0, 0.3);\n"
"padding: 10px 20px;\n"
"}\n"
"QComboBox,QSpinBox,QDateEdit {\n"
"    font-size: 16px;\n"
"    padding: 6px;\n"
"    border: 2px solid #59abc8;\n"
"    border-radius: 15px;\n"
"	background-color: rgba(0, 0, 50, 0.8); \n"
"}\n"
"QPushButton:hover {\n"
"    transform: translateY(-2px);\n"
"    color: #59abc8;\n"
"}\n"
"#label_9c,#label_14c,#label_15c,#label_11c,#label_12c,#label_18c,#label_3c{border:none;\n"
"font-size: 15px;   \n"
"    padding: 1px;\n"
"background-color:transparent;}\n"
"QFrame {\n"
"     border-radius: 15px;\n"
" 		backgroun"
                        "d-color: rgba(0, 0, 50, 0.7); \n"
"\n"
"\n"
"    border: 2px solid #59abc8 ; \n"
"}\n"
"\n"
"#lineEdit{\n"
"border: 2px solid #59abc8 ; \n"
"	background-color: rgba(0, 0, 50, 0.7); }\n"
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
"} \n"
"#pagec{background-image: url(:/new/prefix1/bg.png);\n"
" background-repeat: no-repeat;\n"
"    background-position: center;\n"
"    background-size: cover;\n"
"border-radius:15px;}\n"
"/* TableWidget */\n"
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
"QTableWidget::item:selecte"
                        "d {\n"
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
""));
        pagec = new QWidget();
        pagec->setObjectName("pagec");
        label_6c = new QLabel(pagec);
        label_6c->setObjectName("label_6c");
        label_6c->setGeometry(QRect(100, 120, 121, 31));
        frame_2c = new QFrame(pagec);
        frame_2c->setObjectName("frame_2c");
        frame_2c->setGeometry(QRect(10, 160, 291, 481));
        frame_2c->setFrameShape(QFrame::Shape::StyledPanel);
        frame_2c->setFrameShadow(QFrame::Shadow::Raised);
        lineEdit_3c = new QLineEdit(frame_2c);
        lineEdit_3c->setObjectName("lineEdit_3c");
        lineEdit_3c->setGeometry(QRect(20, 40, 231, 28));
        lineEdit_4c = new QLineEdit(frame_2c);
        lineEdit_4c->setObjectName("lineEdit_4c");
        lineEdit_4c->setGeometry(QRect(20, 90, 231, 28));
        comboBox_2c = new QComboBox(frame_2c);
        comboBox_2c->addItem(QString());
        comboBox_2c->addItem(QString());
        comboBox_2c->addItem(QString());
        comboBox_2c->setObjectName("comboBox_2c");
        comboBox_2c->setEnabled(true);
        comboBox_2c->setGeometry(QRect(120, 250, 151, 41));
        comboBox_2c->setEditable(true);
        label_14c = new QLabel(frame_2c);
        label_14c->setObjectName("label_14c");
        label_14c->setGeometry(QRect(20, 260, 81, 20));
        lineEdit_12c = new QLineEdit(frame_2c);
        lineEdit_12c->setObjectName("lineEdit_12c");
        lineEdit_12c->setGeometry(QRect(20, 140, 231, 28));
        label_9c = new QLabel(frame_2c);
        label_9c->setObjectName("label_9c");
        label_9c->setGeometry(QRect(20, 200, 121, 20));
        comboBoxc = new QComboBox(frame_2c);
        comboBoxc->addItem(QString());
        comboBoxc->addItem(QString());
        comboBoxc->setObjectName("comboBoxc");
        comboBoxc->setEnabled(true);
        comboBoxc->setGeometry(QRect(120, 190, 151, 41));
        comboBoxc->setEditable(true);
        comboBoxc->setDuplicatesEnabled(false);
        dateEdit_c = new QDateEdit(frame_2c);
        dateEdit_c->setObjectName("dateEdit_c");
        dateEdit_c->setGeometry(QRect(120, 360, 141, 41));
        label_18c = new QLabel(frame_2c);
        label_18c->setObjectName("label_18c");
        label_18c->setGeometry(QRect(30, 370, 81, 20));
        pushButton_2c = new QPushButton(frame_2c);
        pushButton_2c->setObjectName("pushButton_2c");
        pushButton_2c->setGeometry(QRect(90, 430, 101, 61));
        lineEdit_14c = new QLineEdit(frame_2c);
        lineEdit_14c->setObjectName("lineEdit_14c");
        lineEdit_14c->setGeometry(QRect(20, 310, 231, 28));
        tableWidgetc = new QTableWidget(pagec);
        if (tableWidgetc->columnCount() < 9)
            tableWidgetc->setColumnCount(9);
        QFont font;
        font.setBold(true);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        __qtablewidgetitem->setFont(font);
        tableWidgetc->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        __qtablewidgetitem1->setFont(font);
        tableWidgetc->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        __qtablewidgetitem2->setFont(font);
        tableWidgetc->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        __qtablewidgetitem3->setFont(font);
        tableWidgetc->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        __qtablewidgetitem4->setFont(font);
        tableWidgetc->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        __qtablewidgetitem5->setFont(font);
        tableWidgetc->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        __qtablewidgetitem6->setFont(font);
        tableWidgetc->setHorizontalHeaderItem(6, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        __qtablewidgetitem7->setFont(font);
        tableWidgetc->setHorizontalHeaderItem(7, __qtablewidgetitem7);
        QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
        __qtablewidgetitem8->setFont(font);
        tableWidgetc->setHorizontalHeaderItem(8, __qtablewidgetitem8);
        tableWidgetc->setObjectName("tableWidgetc");
        tableWidgetc->setGeometry(QRect(310, 150, 1131, 241));
        tableWidgetc->horizontalHeader()->setDefaultSectionSize(125);
        label_3c = new QLabel(pagec);
        label_3c->setObjectName("label_3c");
        label_3c->setGeometry(QRect(350, 100, 41, 51));
        lineEdit_7c = new QLineEdit(pagec);
        lineEdit_7c->setObjectName("lineEdit_7c");
        lineEdit_7c->setGeometry(QRect(390, 110, 131, 28));
        label_12c = new QLabel(pagec);
        label_12c->setObjectName("label_12c");
        label_12c->setGeometry(QRect(830, 110, 63, 20));
        pushButton_5c = new QPushButton(pagec);
        pushButton_5c->setObjectName("pushButton_5c");
        pushButton_5c->setGeometry(QRect(1380, 230, 61, 61));
        pushButton_5c->setStyleSheet(QString::fromUtf8("border:none;\n"
"background-color:transparent;"));
        frame_c = new QFrame(pagec);
        frame_c->setObjectName("frame_c");
        frame_c->setGeometry(QRect(320, 400, 421, 281));
        frame_c->setStyleSheet(QString::fromUtf8("background-color: rgba(0, 0, 50, 0.7);\n"
"        border: 2px solid #59abc8;\n"
"        border-radius: 15px;"));
        frame_c->setFrameShape(QFrame::Shape::StyledPanel);
        frame_c->setFrameShadow(QFrame::Shadow::Raised);
        verticalLayout_zones = new QVBoxLayout(frame_c);
        verticalLayout_zones->setObjectName("verticalLayout_zones");
        label_zonesc = new QLabel(frame_c);
        label_zonesc->setObjectName("label_zonesc");
        label_zonesc->setStyleSheet(QString::fromUtf8("color: #f3ffbe;\n"
"                    font-weight: bold;\n"
"                    font-size: 17px;\n"
"                    padding: 10px;"));
        label_zonesc->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout_zones->addWidget(label_zonesc);

        horizontalLayout_c = new QHBoxLayout();
        horizontalLayout_c->setObjectName("horizontalLayout_c");
        Layou_1c = new QVBoxLayout();
        Layou_1c->setObjectName("Layou_1c");
        label_Cc = new QLabel(frame_c);
        label_Cc->setObjectName("label_Cc");
        label_Cc->setStyleSheet(QString::fromUtf8("color: white;\n"
"                                    text-align: center;\n"
"                                    font-weight: bold;"));
        label_Cc->setAlignment(Qt::AlignmentFlag::AlignCenter);

        Layou_1c->addWidget(label_Cc);

        progressCc = new QProgressBar(frame_c);
        progressCc->setObjectName("progressCc");
        progressCc->setMinimumSize(QSize(30, 120));
        progressCc->setStyleSheet(QString::fromUtf8("QProgressBar:vertical {\n"
"    width: 25px;\n"
"    border: 2px solid #3498db;\n"
"    border-radius: 5px;\n"
"    background-color: rgba(0, 40, 80, 0.5);\n"
"    text-align: center;\n"
"    padding: 1px;\n"
"\n"
"}\n"
"QProgressBar::chunk:vertical {\n"
"    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,\n"
"        stop:0 #3498db, stop:1 #2980b9);\n"
"    border-radius: 3px;\n"
"}"));
        progressCc->setValue(0);
        progressCc->setTextVisible(true);
        progressCc->setOrientation(Qt::Orientation::Vertical);

        Layou_1c->addWidget(progressCc);

        value_Cc = new QLabel(frame_c);
        value_Cc->setObjectName("value_Cc");
        value_Cc->setStyleSheet(QString::fromUtf8("color: #3498db;\n"
"                                    font-weight: bold;\n"
"                                    font-size: 12px;\n"
"                                    text-align: center;"));
        value_Cc->setAlignment(Qt::AlignmentFlag::AlignCenter);

        Layou_1c->addWidget(value_Cc);


        horizontalLayout_c->addLayout(Layou_1c);

        Layout_CCc = new QVBoxLayout();
        Layout_CCc->setObjectName("Layout_CCc");
        label_CCc = new QLabel(frame_c);
        label_CCc->setObjectName("label_CCc");
        label_CCc->setStyleSheet(QString::fromUtf8("color: white;\n"
"                                    text-align: center;\n"
"                                    font-weight: bold;"));

        Layout_CCc->addWidget(label_CCc);

        progressCCc = new QProgressBar(frame_c);
        progressCCc->setObjectName("progressCCc");
        progressCCc->setMinimumSize(QSize(30, 120));
        progressCCc->setStyleSheet(QString::fromUtf8("QProgressBar:vertical {\n"
"    width: 25px;\n"
"    border: 2px solid #27ae60;\n"
"    border-radius: 5px;\n"
"    background-color: rgba(0, 40, 80, 0.5);\n"
"    text-align: center;\n"
"    padding: 1px;\n"
"}\n"
"QProgressBar::chunk:vertical {\n"
"    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,\n"
"        stop:0 #27ae60, stop:1 #229954);\n"
"    border-radius: 3px;\n"
"}"));
        progressCCc->setValue(0);
        progressCCc->setTextVisible(true);
        progressCCc->setOrientation(Qt::Orientation::Vertical);

        Layout_CCc->addWidget(progressCCc);

        value_CCc = new QLabel(frame_c);
        value_CCc->setObjectName("value_CCc");
        value_CCc->setStyleSheet(QString::fromUtf8("color: #27ae60;\n"
"                                    font-weight: bold;\n"
"                                    font-size: 12px;\n"
"                                    text-align: center;"));

        Layout_CCc->addWidget(value_CCc);


        horizontalLayout_c->addLayout(Layout_CCc);

        Layoutc = new QVBoxLayout();
        Layoutc->setObjectName("Layoutc");
        label_c = new QLabel(frame_c);
        label_c->setObjectName("label_c");
        label_c->setStyleSheet(QString::fromUtf8("color: white;\n"
"                                    text-align: center;\n"
"                                    font-weight: bold;"));

        Layoutc->addWidget(label_c);

        progressc = new QProgressBar(frame_c);
        progressc->setObjectName("progressc");
        progressc->setMinimumSize(QSize(30, 120));
        progressc->setStyleSheet(QString::fromUtf8("QProgressBar:vertical {\n"
"    width: 25px;\n"
"    border: 2px solid #f39c12;\n"
"    border-radius: 5px;\n"
"    background-color: rgba(0, 40, 80, 0.5);\n"
"    text-align: center;\n"
"    padding: 1px;\n"
"}\n"
"QProgressBar::chunk:vertical {\n"
"    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,\n"
"        stop:0 #f39c12, stop:1 #d68910);\n"
"    border-radius: 3px;\n"
"}"));
        progressc->setValue(0);
        progressc->setTextVisible(true);
        progressc->setOrientation(Qt::Orientation::Vertical);

        Layoutc->addWidget(progressc);

        value_c = new QLabel(frame_c);
        value_c->setObjectName("value_c");
        value_c->setStyleSheet(QString::fromUtf8("color: #f39c12;\n"
"                                    font-weight: bold;\n"
"                                    font-size: 12px;\n"
"                                    text-align: center;"));

        Layoutc->addWidget(value_c);


        horizontalLayout_c->addLayout(Layoutc);


        verticalLayout_zones->addLayout(horizontalLayout_c);

        pushButton_7c = new QPushButton(pagec);
        pushButton_7c->setObjectName("pushButton_7c");
        pushButton_7c->setGeometry(QRect(20, 730, 141, 61));
        pushButton_8c = new QPushButton(pagec);
        pushButton_8c->setObjectName("pushButton_8c");
        pushButton_8c->setGeometry(QRect(1350, 710, 161, 61));
        label_5c = new QLabel(pagec);
        label_5c->setObjectName("label_5c");
        label_5c->setGeometry(QRect(600, 20, 431, 61));
        dateEdit_2c = new QDateEdit(pagec);
        dateEdit_2c->setObjectName("dateEdit_2c");
        dateEdit_2c->setGeometry(QRect(890, 100, 131, 41));
        pushButton_6c = new QPushButton(pagec);
        pushButton_6c->setObjectName("pushButton_6c");
        pushButton_6c->setGeometry(QRect(1320, 230, 61, 61));
        pushButton_6c->setStyleSheet(QString::fromUtf8("border:none;\n"
"background-color:transparent;"));
        pushButton_9c = new QPushButton(pagec);
        pushButton_9c->setObjectName("pushButton_9c");
        pushButton_9c->setGeometry(QRect(1350, 230, 61, 61));
        pushButton_9c->setStyleSheet(QString::fromUtf8("border:none;\n"
"background-color:transparent;"));
        label_11c = new QLabel(pagec);
        label_11c->setObjectName("label_11c");
        label_11c->setGeometry(QRect(550, 110, 121, 20));
        comboBoxc_2 = new QComboBox(pagec);
        comboBoxc_2->addItem(QString());
        comboBoxc_2->addItem(QString());
        comboBoxc_2->setObjectName("comboBoxc_2");
        comboBoxc_2->setEnabled(true);
        comboBoxc_2->setGeometry(QRect(650, 100, 151, 41));
        comboBoxc_2->setEditable(true);
        comboBoxc_2->setDuplicatesEnabled(false);
        btnai = new QPushButton(pagec);
        btnai->setObjectName("btnai");
        btnai->setGeometry(QRect(1450, 10, 61, 64));
        frame_2c_2 = new QFrame(pagec);
        frame_2c_2->setObjectName("frame_2c_2");
        frame_2c_2->setGeometry(QRect(1110, 10, 401, 561));
        frame_2c_2->setFrameShape(QFrame::Shape::StyledPanel);
        frame_2c_2->setFrameShadow(QFrame::Shadow::Raised);
        textChatHistory = new QTextEdit(frame_2c_2);
        textChatHistory->setObjectName("textChatHistory");
        textChatHistory->setGeometry(QRect(10, 10, 381, 391));
        textChatHistory->setStyleSheet(QString::fromUtf8("border:none;\n"
"border-bottom:1px solid #94f6ff;"));
        textChatHistory->setReadOnly(true);
        frame_2c_3 = new QFrame(frame_2c_2);
        frame_2c_3->setObjectName("frame_2c_3");
        frame_2c_3->setGeometry(QRect(10, 410, 371, 81));
        frame_2c_3->setStyleSheet(QString::fromUtf8(""));
        frame_2c_3->setFrameShape(QFrame::Shape::StyledPanel);
        frame_2c_3->setFrameShadow(QFrame::Shadow::Raised);
        lineEdit_3c_2 = new QLineEdit(frame_2c_3);
        lineEdit_3c_2->setObjectName("lineEdit_3c_2");
        lineEdit_3c_2->setGeometry(QRect(20, 30, 331, 28));
        btnSend_2 = new QPushButton(frame_2c_2);
        btnSend_2->setObjectName("btnSend_2");
        btnSend_2->setGeometry(QRect(200, 500, 101, 64));
        btnSend_2->setStyleSheet(QString::fromUtf8("background-color: rgba(120, 0, 0,0.6);"));
        btnSend = new QPushButton(frame_2c_2);
        btnSend->setObjectName("btnSend");
        btnSend->setGeometry(QRect(90, 500, 101, 64));
        label_6c_2 = new QLabel(pagec);
        label_6c_2->setObjectName("label_6c_2");
        label_6c_2->setGeometry(QRect(770, 400, 211, 31));
        label_6c_2->setStyleSheet(QString::fromUtf8("\n"
"font: 700 9.5pt \"Segoe UI\";"));
        tableWidget = new QTableWidget(pagec);
        if (tableWidget->columnCount() < 4)
            tableWidget->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem9 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(0, __qtablewidgetitem9);
        QTableWidgetItem *__qtablewidgetitem10 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(1, __qtablewidgetitem10);
        QTableWidgetItem *__qtablewidgetitem11 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(2, __qtablewidgetitem11);
        QTableWidgetItem *__qtablewidgetitem12 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(3, __qtablewidgetitem12);
        if (tableWidget->rowCount() < 5)
            tableWidget->setRowCount(5);
        QTableWidgetItem *__qtablewidgetitem13 = new QTableWidgetItem();
        tableWidget->setItem(0, 0, __qtablewidgetitem13);
        QTableWidgetItem *__qtablewidgetitem14 = new QTableWidgetItem();
        tableWidget->setItem(0, 1, __qtablewidgetitem14);
        QTableWidgetItem *__qtablewidgetitem15 = new QTableWidgetItem();
        tableWidget->setItem(0, 2, __qtablewidgetitem15);
        QTableWidgetItem *__qtablewidgetitem16 = new QTableWidgetItem();
        tableWidget->setItem(0, 3, __qtablewidgetitem16);
        tableWidget->setObjectName("tableWidget");
        tableWidget->setGeometry(QRect(760, 440, 541, 231));
        tableWidget->horizontalHeader()->setMinimumSectionSize(30);
        tableWidget->horizontalHeader()->setDefaultSectionSize(125);
        pushButton_8c_2 = new QPushButton(pagec);
        pushButton_8c_2->setObjectName("pushButton_8c_2");
        pushButton_8c_2->setGeometry(QRect(960, 680, 161, 61));
        pushButton_8c_2->setStyleSheet(QString::fromUtf8("background-color: rgba(0, 170, 0,0.6);"));
        stackedWidgetc->addWidget(pagec);
        tableWidget->raise();
        frame_c->raise();
        label_6c->raise();
        frame_2c->raise();
        tableWidgetc->raise();
        label_3c->raise();
        lineEdit_7c->raise();
        label_12c->raise();
        pushButton_5c->raise();
        pushButton_7c->raise();
        pushButton_8c->raise();
        label_5c->raise();
        dateEdit_2c->raise();
        pushButton_6c->raise();
        pushButton_9c->raise();
        label_11c->raise();
        comboBoxc_2->raise();
        frame_2c_2->raise();
        btnai->raise();
        label_6c_2->raise();
        pushButton_8c_2->raise();
        page_2 = new QWidget();
        page_2->setObjectName("page_2");
        stackedWidgetc->addWidget(page_2);
        MainWindow->setCentralWidget(centralwidgetc);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1573, 25));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        label_6c->setText(QCoreApplication::translate("MainWindow", "  Formulaire", nullptr));
        lineEdit_3c->setText(QString());
        lineEdit_3c->setPlaceholderText(QCoreApplication::translate("MainWindow", "ID client", nullptr));
        lineEdit_4c->setPlaceholderText(QCoreApplication::translate("MainWindow", "Nom client", nullptr));
        comboBox_2c->setItemText(0, QCoreApplication::translate("MainWindow", "Fidele", nullptr));
        comboBox_2c->setItemText(1, QCoreApplication::translate("MainWindow", "Occasionnel", nullptr));
        comboBox_2c->setItemText(2, QCoreApplication::translate("MainWindow", "Regulier", nullptr));

        comboBox_2c->setCurrentText(QCoreApplication::translate("MainWindow", "Fidele", nullptr));
        label_14c->setText(QCoreApplication::translate("MainWindow", "Profil Client", nullptr));
        lineEdit_12c->setPlaceholderText(QCoreApplication::translate("MainWindow", "Prenom client", nullptr));
        label_9c->setText(QCoreApplication::translate("MainWindow", "Statut Client", nullptr));
        comboBoxc->setItemText(0, QCoreApplication::translate("MainWindow", "Conforme", nullptr));
        comboBoxc->setItemText(1, QCoreApplication::translate("MainWindow", "Non conforme", nullptr));

        comboBoxc->setCurrentText(QCoreApplication::translate("MainWindow", "Conforme", nullptr));
        comboBoxc->setPlaceholderText(QString());
        label_18c->setText(QCoreApplication::translate("MainWindow", "Date", nullptr));
        pushButton_2c->setText(QCoreApplication::translate("MainWindow", "Ajouter", nullptr));
        lineEdit_14c->setPlaceholderText(QCoreApplication::translate("MainWindow", "Telephone", nullptr));
        QTableWidgetItem *___qtablewidgetitem = tableWidgetc->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("MainWindow", "Id Client", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = tableWidgetc->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("MainWindow", " Nom Client", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = tableWidgetc->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("MainWindow", "Prenom Client", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = tableWidgetc->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("MainWindow", "Statut Client", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = tableWidgetc->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("MainWindow", "Profil Client ", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = tableWidgetc->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QCoreApplication::translate("MainWindow", "Statut Client", nullptr));
        QTableWidgetItem *___qtablewidgetitem6 = tableWidgetc->horizontalHeaderItem(6);
        ___qtablewidgetitem6->setText(QCoreApplication::translate("MainWindow", "Date", nullptr));
        QTableWidgetItem *___qtablewidgetitem7 = tableWidgetc->horizontalHeaderItem(7);
        ___qtablewidgetitem7->setText(QCoreApplication::translate("MainWindow", "Telephone", nullptr));
        QTableWidgetItem *___qtablewidgetitem8 = tableWidgetc->horizontalHeaderItem(8);
        ___qtablewidgetitem8->setText(QCoreApplication::translate("MainWindow", "Action", nullptr));
        label_3c->setText(QCoreApplication::translate("MainWindow", "\360\237\224\215:", nullptr));
        lineEdit_7c->setPlaceholderText(QCoreApplication::translate("MainWindow", "Recherche", nullptr));
        label_12c->setText(QCoreApplication::translate("MainWindow", "Date:", nullptr));
        pushButton_5c->setText(QCoreApplication::translate("MainWindow", "\342\234\217\357\270\217", nullptr));
        label_zonesc->setText(QCoreApplication::translate("MainWindow", " Statistiques par Profil Client", nullptr));
        label_Cc->setText(QCoreApplication::translate("MainWindow", "Fidele", nullptr));
        progressCc->setFormat(QCoreApplication::translate("MainWindow", "%p%", nullptr));
        value_Cc->setText(QCoreApplication::translate("MainWindow", "0/0", nullptr));
        label_CCc->setText(QCoreApplication::translate("MainWindow", "Occasionnel", nullptr));
        progressCCc->setFormat(QCoreApplication::translate("MainWindow", "%p%", nullptr));
        value_CCc->setText(QCoreApplication::translate("MainWindow", "0/0", nullptr));
        label_c->setText(QCoreApplication::translate("MainWindow", "Regulier", nullptr));
        progressc->setFormat(QCoreApplication::translate("MainWindow", "%p%", nullptr));
        value_c->setText(QCoreApplication::translate("MainWindow", "0/0", nullptr));
        pushButton_7c->setText(QCoreApplication::translate("MainWindow", "Retour Menu", nullptr));
        pushButton_8c->setText(QCoreApplication::translate("MainWindow", "\360\237\223\204Exporter PDF", nullptr));
        label_5c->setText(QCoreApplication::translate("MainWindow", "Gestion des clients", nullptr));
        pushButton_6c->setText(QCoreApplication::translate("MainWindow", "\360\237\224\204", nullptr));
        pushButton_9c->setText(QCoreApplication::translate("MainWindow", "\342\235\214", nullptr));
        label_11c->setText(QCoreApplication::translate("MainWindow", "Statut Client:", nullptr));
        comboBoxc_2->setItemText(0, QCoreApplication::translate("MainWindow", "Conforme", nullptr));
        comboBoxc_2->setItemText(1, QCoreApplication::translate("MainWindow", "Non conforme", nullptr));

        comboBoxc_2->setCurrentText(QCoreApplication::translate("MainWindow", "Conforme", nullptr));
        comboBoxc_2->setPlaceholderText(QString());
        btnai->setText(QCoreApplication::translate("MainWindow", "\360\237\244\226", nullptr));
        lineEdit_3c_2->setText(QString());
        lineEdit_3c_2->setPlaceholderText(QCoreApplication::translate("MainWindow", "Ecrire question", nullptr));
        btnSend_2->setText(QCoreApplication::translate("MainWindow", "Fermer", nullptr));
        btnSend->setText(QCoreApplication::translate("MainWindow", "Envoyer", nullptr));
        label_6c_2->setText(QCoreApplication::translate("MainWindow", "\360\237\217\206 Top 5 Clients Fid\303\250les", nullptr));
        QTableWidgetItem *___qtablewidgetitem9 = tableWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem9->setText(QCoreApplication::translate("MainWindow", "#", nullptr));
        QTableWidgetItem *___qtablewidgetitem10 = tableWidget->horizontalHeaderItem(1);
        ___qtablewidgetitem10->setText(QCoreApplication::translate("MainWindow", "Client", nullptr));
        QTableWidgetItem *___qtablewidgetitem11 = tableWidget->horizontalHeaderItem(2);
        ___qtablewidgetitem11->setText(QCoreApplication::translate("MainWindow", "Score", nullptr));
        QTableWidgetItem *___qtablewidgetitem12 = tableWidget->horizontalHeaderItem(3);
        ___qtablewidgetitem12->setText(QCoreApplication::translate("MainWindow", "Reduction", nullptr));

        const bool __sortingEnabled = tableWidget->isSortingEnabled();
        tableWidget->setSortingEnabled(false);
        QTableWidgetItem *___qtablewidgetitem13 = tableWidget->item(0, 0);
        ___qtablewidgetitem13->setText(QCoreApplication::translate("MainWindow", "1", nullptr));
        QTableWidgetItem *___qtablewidgetitem14 = tableWidget->item(0, 1);
        ___qtablewidgetitem14->setText(QCoreApplication::translate("MainWindow", "fourat attia", nullptr));
        QTableWidgetItem *___qtablewidgetitem15 = tableWidget->item(0, 2);
        ___qtablewidgetitem15->setText(QCoreApplication::translate("MainWindow", "90%", nullptr));
        QTableWidgetItem *___qtablewidgetitem16 = tableWidget->item(0, 3);
        ___qtablewidgetitem16->setText(QCoreApplication::translate("MainWindow", "-50%", nullptr));
        tableWidget->setSortingEnabled(__sortingEnabled);

        pushButton_8c_2->setText(QCoreApplication::translate("MainWindow", "Envoyer SMS", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GCLIENTS_H
