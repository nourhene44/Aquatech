QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    GQuai.cpp \
    captures.cpp \
    connection.cpp \
    quotas.cpp \
    actiondelegate.cpp \
    actionswidget.cpp \
    main.cpp

HEADERS += \
    GQuai.h \
    captures.h \
    connection.h \
    quotas.h \
    actiondelegate.h \
    actionswidget.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    img.qrc

# 添加这些行以确保正确的编译
DEFINES += QT_DEPRECATED_WARNINGS

# Export Excel réel (.xlsx) via ActiveQt/COM (nécessite Excel installé)
win32:qtHaveModule(axcontainer) {
    QT += axcontainer
    DEFINES += HAVE_ACTIVEQT
}

DISTFILES += \
    modifier-le-fichier (1).png
