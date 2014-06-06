#-------------------------------------------------
#
# Project created by QtCreator 2013-12-13T22:02:01
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = ModbusScope
TEMPLATE = app

DEFINES += QT_DEBUG_OUTPUT

INCLUDEPATH += \
    ../libraries/libmodbus/src \
    ../libraries/qcustomplot \
    ../src

SOURCES +=  \
    ../src/mainwindow.cpp \
    ../src/settingsdialog.cpp \
    ../src/modbussettings.cpp \
    ../src/main.cpp \
    ../src/modbusmaster.cpp \
    ../libraries/libmodbus/src/modbus-data.c \
    ../libraries/libmodbus/src/modbus.c \
    ../libraries/libmodbus/src/modbus-tcp.c \
    ../libraries/qcustomplot/qcustomplot.cpp \
    ../src/modbusscope.cpp \
    ../src/scopegui.cpp

FORMS    += \
    ../src/mainwindow.ui \
    ../src/settingsdialog.ui

HEADERS += \
    ../src/mainwindow.h \
    ../src/settingsdialog.h \
    ../src/modbussettings.h \
    ../src/modbusmaster.h \
    ../libraries/libmodbus/src/modbus-private.h \
    ../libraries/libmodbus/src/modbus-version.h \
    ../libraries/libmodbus/src/modbus.h \
    ../libraries/libmodbus/src/modbus-tcp.h \
    ../libraries/libmodbus/src/modbus-tcp-private.h \
    ../libraries/libmodbus/src/config.h \
    ../libraries/qcustomplot/qcustomplot.h \
    ../src/modbusscope.h \
    ../src/scopegui.h
