/********************************************************************************
** Form generated from reading UI file 'tendance.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TENDANCE_H
#define UI_TENDANCE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>

QT_BEGIN_NAMESPACE

class Ui_Dialog
{
public:
    QGroupBox *groupStatsBateau;
    QHBoxLayout *layoutStatsBateau;
    QLabel *label_idBateau;
    QLabel *lblIdBateauValue;
    QSpacerItem *spacerStatsBateau;
    QLabel *label;
    QFrame *frame;
    QComboBox *comboBox;
    QLabel *label_2;
    QLabel *label_3;
    QDateEdit *dateEdit;
    QLabel *label_4;
    QDateEdit *dateEdit_2;
    QPushButton *pushButton;
    QLabel *label_5;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName("Dialog");
        Dialog->resize(1528, 856);
        groupStatsBateau = new QGroupBox(Dialog);
        groupStatsBateau->setObjectName("groupStatsBateau");
        layoutStatsBateau = new QHBoxLayout(groupStatsBateau);
        layoutStatsBateau->setObjectName("layoutStatsBateau");
        label_idBateau = new QLabel(groupStatsBateau);
        label_idBateau->setObjectName("label_idBateau");
        label_idBateau->setStyleSheet(QString::fromUtf8("font-weight: bold; color: white;"));

        layoutStatsBateau->addWidget(label_idBateau);

        lblIdBateauValue = new QLabel(groupStatsBateau);
        lblIdBateauValue->setObjectName("lblIdBateauValue");
        lblIdBateauValue->setStyleSheet(QString::fromUtf8("color: #59abc8; font-weight: bold; font-size: 16px;"));

        layoutStatsBateau->addWidget(lblIdBateauValue);

        spacerStatsBateau = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        layoutStatsBateau->addItem(spacerStatsBateau);

        label = new QLabel(Dialog);
        label->setObjectName("label");
        label->setGeometry(QRect(590, 40, 171, 61));
        frame = new QFrame(Dialog);
        frame->setObjectName("frame");
        frame->setGeometry(QRect(200, 100, 1121, 631));
        frame->setFrameShape(QFrame::Shape::StyledPanel);
        frame->setFrameShadow(QFrame::Shadow::Raised);
        comboBox = new QComboBox(frame);
        comboBox->setObjectName("comboBox");
        comboBox->setGeometry(QRect(380, 20, 171, 41));
        label_2 = new QLabel(frame);
        label_2->setObjectName("label_2");
        label_2->setGeometry(QRect(30, 10, 91, 61));
        label_3 = new QLabel(frame);
        label_3->setObjectName("label_3");
        label_3->setGeometry(QRect(560, 10, 91, 61));
        dateEdit = new QDateEdit(frame);
        dateEdit->setObjectName("dateEdit");
        dateEdit->setGeometry(QRect(660, 20, 121, 51));
        label_4 = new QLabel(frame);
        label_4->setObjectName("label_4");
        label_4->setGeometry(QRect(790, 20, 21, 41));
        dateEdit_2 = new QDateEdit(frame);
        dateEdit_2->setObjectName("dateEdit_2");
        dateEdit_2->setGeometry(QRect(810, 20, 121, 51));
        pushButton = new QPushButton(frame);
        pushButton->setObjectName("pushButton");
        pushButton->setGeometry(QRect(950, 30, 93, 29));
        label_5 = new QLabel(frame);
        label_5->setObjectName("label_5");
        label_5->setGeometry(QRect(300, 10, 61, 61));

        retranslateUi(Dialog);

        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QCoreApplication::translate("Dialog", "Dialog", nullptr));
        groupStatsBateau->setTitle(QCoreApplication::translate("Dialog", "Stats Bateau", nullptr));
        label_idBateau->setText(QCoreApplication::translate("Dialog", "ID BATEAU :", nullptr));
        lblIdBateauValue->setText(QCoreApplication::translate("Dialog", "---", nullptr));
        label->setText(QCoreApplication::translate("Dialog", "TENDANCE SAISONIERE ", nullptr));
        label_2->setText(QCoreApplication::translate("Dialog", "ID BATEAU :", nullptr));
        label_3->setText(QCoreApplication::translate("Dialog", "PERIODE : du", nullptr));
        label_4->setText(QCoreApplication::translate("Dialog", "au", nullptr));
        pushButton->setText(QCoreApplication::translate("Dialog", "AFFICHER", nullptr));
        label_5->setText(QCoreApplication::translate("Dialog", "ESPECE :", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Dialog: public Ui_Dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TENDANCE_H
