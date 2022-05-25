#-------------------------------------------------
#
# Project created by QtCreator 2012-04-21T09:28:41
#
#-------------------------------------------------

QT       += core gui network widgets serialport
#QT       += core gui widgets

TARGET = GCoder
TEMPLATE = app

# include(qextserialport/src/qextserialport.pri)
RESOURCES   += \
               gcoder.qrc

SOURCES +=  main.cpp\
            executionstate.cpp \
            gchttpservice.cpp \
            mainwindow.cpp \
            XSettings.cpp \
            SweepDialog.cpp \
            QcjData/QcjHttpService.cpp \
            QcjData/QcjTcpService.cpp \
            GCode.cpp

HEADERS  += \
            executionstate.h \
            gchttpservice.h \
            GCHttpServiceFactory.h \
            mainwindow.h \
            pausemessagedialog.h \
            outputdeviceselection.h \
            XSettings.h \
            SweepDialog.h \
            xlineedit.h \
            QcjData/QcjHttpService.h \
            QcjData/QcjHttpServiceFactory.h \
            QcjData/QcjTcpService.h \
            GCode.h

FORMS += \
            outputdeviceselection.ui \
            pausemessagedialog.ui \
            sweep_dialog.ui \
            mainwindow.ui \


# unix:!macx:!symbian: LIBS += -lSerialPort
