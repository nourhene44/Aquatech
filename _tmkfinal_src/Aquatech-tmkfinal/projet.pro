QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    captures.cpp \
    client.cpp \
    connection.cpp \
    employe.cpp \
    main.cpp \
    mainwindow.cpp \
    quai.cpp \
    bateaauuu.cpp \
    pecheurs.cpp

HEADERS += \
    captures.h \
    client.h \
    connection.h \
    employe.h \
    mainwindow.h \
    quai.h \
    bateaauuu.h \
    pecheurs.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    img.qrc

DEFINES += QT_DEPRECATED_WARNINGS

# Export Excel réel (.xlsx) via ActiveQt/COM (nécessite Excel installé)
win32:qtHaveModule(axcontainer) {
    QT += axcontainer
    DEFINES += HAVE_ACTIVEQT
}

DISTFILES += \
    modifier-le-fichier (1).png
