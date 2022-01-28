TARGET = file-tracker
TEMPLATE = app
CONFIG += c++11
QT += core dtkcore
QT -= gui

isEmpty(VERSION): VERSION = 0.1.0

DEFINES += QT_MESSAGELOGCONTEX

SOURCES += \
    src/main.cpp \
    src/server.cpp \
    src/tracesmoke.cpp

INCLUDEPATH += ../kernelmod

HEADERS += \
    src/server.h \
    src/tracesmoke.h

isEmpty(PREFIX): PREFIX = /usr

target.path = $$PREFIX/bin

systemd_service.files = $${TARGET}.service
systemd_service.path = /lib/systemd/system

INSTALLS += target systemd_service sysusers
