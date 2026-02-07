/********************************************************************************
** Form generated from reading UI file 'surpeche.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SURPECHE_H
#define UI_SURPECHE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextBrowser>

QT_BEGIN_NAMESPACE

class Ui_surpeche
{
public:
    QLabel *lblTitle;
    QFrame *frame;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QTableWidget *tableWidget;
    QLabel *label_5;
    QGroupBox *groupBox;
    QLabel *label_6;
    QTextBrowser *textBrowser;
    QGroupBox *groupBox_2;
    QProgressBar *progressBar;
    QProgressBar *progressBar_2;
    QProgressBar *progressBar_3;
    QLabel *label_7;
    QLabel *label_8;
    QLabel *label_9;
    QPushButton *pushButton;

    void setupUi(QDialog *surpeche)
    {
        if (surpeche->objectName().isEmpty())
            surpeche->setObjectName("surpeche");
        surpeche->resize(1372, 826);
        surpeche->setStyleSheet(QString::fromUtf8("QComboBox,QSpinBox,QDoubleSpinBox,QDateEdit {\n"
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
"#label_5,#label_2,#label_3,#label_7,#label_8,#label_9,#label_4{\n"
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
"  "
                        "  color: white;\n"
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
        lblTitle = new QLabel(surpeche);
        lblTitle->setObjectName("lblTitle");
        lblTitle->setGeometry(QRect(430, 20, 361, 71));
        lblTitle->setStyleSheet(QString::fromUtf8("\n"
"font: 900 italic 12pt \"Segoe UI\";"));
        lblTitle->setAlignment(Qt::AlignmentFlag::AlignCenter);
        frame = new QFrame(surpeche);
        frame->setObjectName("frame");
        frame->setGeometry(QRect(120, 100, 951, 721));
        frame->setFrameShape(QFrame::Shape::StyledPanel);
        frame->setFrameShadow(QFrame::Shadow::Raised);
        label_2 = new QLabel(frame);
        label_2->setObjectName("label_2");
        label_2->setGeometry(QRect(20, 0, 101, 41));
        label_3 = new QLabel(frame);
        label_3->setObjectName("label_3");
        label_3->setGeometry(QRect(480, 0, 51, 41));
        label_4 = new QLabel(frame);
        label_4->setObjectName("label_4");
        label_4->setGeometry(QRect(660, 0, 241, 41));
        tableWidget = new QTableWidget(frame);
        if (tableWidget->columnCount() < 6)
            tableWidget->setColumnCount(6);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        tableWidget->setObjectName("tableWidget");
        tableWidget->setGeometry(QRect(80, 70, 761, 311));
        label_5 = new QLabel(frame);
        label_5->setObjectName("label_5");
        label_5->setGeometry(QRect(240, 0, 81, 41));
        groupBox = new QGroupBox(frame);
        groupBox->setObjectName("groupBox");
        groupBox->setGeometry(QRect(30, 400, 531, 241));
        label_6 = new QLabel(groupBox);
        label_6->setObjectName("label_6");
        label_6->setGeometry(QRect(150, 30, 171, 31));
        textBrowser = new QTextBrowser(groupBox);
        textBrowser->setObjectName("textBrowser");
        textBrowser->setGeometry(QRect(10, 70, 471, 161));
        groupBox_2 = new QGroupBox(frame);
        groupBox_2->setObjectName("groupBox_2");
        groupBox_2->setGeometry(QRect(620, 400, 311, 241));
        progressBar = new QProgressBar(groupBox_2);
        progressBar->setObjectName("progressBar");
        progressBar->setGeometry(QRect(160, 70, 131, 21));
        progressBar->setValue(24);
        progressBar_2 = new QProgressBar(groupBox_2);
        progressBar_2->setObjectName("progressBar_2");
        progressBar_2->setGeometry(QRect(160, 120, 131, 21));
        progressBar_2->setValue(24);
        progressBar_3 = new QProgressBar(groupBox_2);
        progressBar_3->setObjectName("progressBar_3");
        progressBar_3->setGeometry(QRect(160, 160, 131, 21));
        progressBar_3->setValue(24);
        label_7 = new QLabel(groupBox_2);
        label_7->setObjectName("label_7");
        label_7->setGeometry(QRect(10, 70, 141, 21));
        label_8 = new QLabel(groupBox_2);
        label_8->setObjectName("label_8");
        label_8->setGeometry(QRect(10, 120, 141, 21));
        label_9 = new QLabel(groupBox_2);
        label_9->setObjectName("label_9");
        label_9->setGeometry(QRect(10, 160, 141, 21));
        pushButton = new QPushButton(frame);
        pushButton->setObjectName("pushButton");
        pushButton->setGeometry(QRect(410, 660, 171, 61));

        retranslateUi(surpeche);

        QMetaObject::connectSlotsByName(surpeche);
    } // setupUi

    void retranslateUi(QDialog *surpeche)
    {
        surpeche->setWindowTitle(QCoreApplication::translate("surpeche", "Dialog", nullptr));
        lblTitle->setText(QCoreApplication::translate("surpeche", "Analyse de Surp\303\252che", nullptr));
        label_2->setText(QCoreApplication::translate("surpeche", "ID CAPTURE :", nullptr));
        label_3->setText(QCoreApplication::translate("surpeche", "DATE :", nullptr));
        label_4->setText(QCoreApplication::translate("surpeche", "NOMBRE D ESPECE CAPTURE :", nullptr));
        QTableWidgetItem *___qtablewidgetitem = tableWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("surpeche", "Esp\303\251ce", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = tableWidget->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("surpeche", "Quantit\303\251", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = tableWidget->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("surpeche", "Poids", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = tableWidget->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("surpeche", "Quotas", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = tableWidget->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("surpeche", "% Utilis\303\251", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = tableWidget->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QCoreApplication::translate("surpeche", "Etat", nullptr));
        label_5->setText(QCoreApplication::translate("surpeche", "BATEAU :", nullptr));
        groupBox->setTitle(QCoreApplication::translate("surpeche", "DIAGNOSTIC", nullptr));
        label_6->setText(QCoreApplication::translate("surpeche", "\303\211tat Globale : ---", nullptr));
        textBrowser->setHtml(QCoreApplication::translate("surpeche", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><meta charset=\"utf-8\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"hr { height: 1px; border-width: 0; }\n"
"li.unchecked::marker { content: \"\\2610\"; }\n"
"li.checked::marker { content: \"\\2612\"; }\n"
"</style></head><body style=\" font-family:'Segoe UI','Century Gothic','sans-serif'; font-size:14px; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Segoe UI'; font-size:9pt; color:#ffffff;\">Analyse en attente...</span></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'Segoe UI'; font-size:9pt; color:#ffffff;\"><br /></p></body></html>", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("surpeche", "INDICATEURS", nullptr));
        label_7->setText(QCoreApplication::translate("surpeche", "NOM ESPECE : ---", nullptr));
        label_8->setText(QCoreApplication::translate("surpeche", "NOM ESPECE : ---", nullptr));
        label_9->setText(QCoreApplication::translate("surpeche", "NOM ESPECE : ---", nullptr));
        pushButton->setText(QCoreApplication::translate("surpeche", "FERMER", nullptr));
    } // retranslateUi

};

namespace Ui {
    class surpeche: public Ui_surpeche {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SURPECHE_H
