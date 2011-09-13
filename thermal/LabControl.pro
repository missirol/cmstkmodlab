# -------------------------------------------------
# Project created by QtCreator 2010-07-30T22:15:00
# -------------------------------------------------
QT += core gui
macx { 
    INCLUDEPATH += /opt/local/include/qwt
    LIBS += -L/opt/local/lib -lqwt 
    LIBS += ../devices/Keithley/*.o ../devices/Greisinger/*.o ../devices/Julabo/*.o ../devices/Hameg/*.o
    DEFINES += USE_FAKEIO
}
else { 
    INCLUDEPATH += /usr/include/qwt
    LIBS += -lqwt -L../devices/lib -lTkModLabKeithley -lTkModLabGreisinger -lTkModLabJulabo -lTkModLabHameg
}
INCLUDEPATH += ../

TARGET = LabControl
TEMPLATE = app
SOURCES += main.cpp \
    labcontrol.cpp \
    labcontroldatacollector.cpp \
    labcontroldata.cpp \
    labcontrolcalibration.cpp \
    labcontrolcommand.cpp \
    labcontrolplot.cpp \
    labcontrolpreferences.cpp
HEADERS += labcontrol.h \
    labcontroldatacollector.h \
    labcontroldata.h \
    labcontrolcalibration.h \
    labcontrolcommand.h \
    labcontrolplot.h \
    labcontrolpreferences.h
FORMS += labcontrol.ui