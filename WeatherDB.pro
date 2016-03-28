#-------------------------------------------------
#
# Project created by QtCreator 2015-12-26T11:58:26
#
#-------------------------------------------------

QT       += core gui sql
#LIBS += -ldbghelp

CONFIG  += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WeatherDB
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
    database.cpp \
    model/dbmodel.cpp \
    model/addrowproxymodel.cpp \
    model/filterproxymodel.cpp \
    model/horizontalproxymodel.cpp \
    view/tabview.cpp \
    view/dbdelegates.cpp \
    view/dbview.cpp \
    view/checkboxheader.cpp \
    view/collapsiblegroupbox.cpp \
    dialogs/changeadddialog.cpp \
	dialogs/customsearchdialog.cpp

HEADERS  += mainwindow.h \
    database.h \
    model/dbmodel.h \
	model/addrowproxymodel.h \
	model/filterproxymodel.h \
	model/horizontalproxymodel.h \
    view/tabview.h \
	view/dbdelegates.h \
    view/dbview.h \
    view/checkboxheader.h \
    view/collapsiblegroupbox.h \
	dialogs/changeadddialog.h \
	dialogs/customsearchdialog.h

VERSION = 0.9.7.0
#QMAKE_TARGET_COMPANY = company
QMAKE_TARGET_PRODUCT = WeatherDB
QMAKE_TARGET_DESCRIPTION = "Weather Database"
#QMAKE_TARGET_COPYRIGHT = copyright

TRANSLATIONS=lang/WeatherDB_ru_RU.ts #lang/default_ru_RU.ts
#don't pass second one to lupdate, it marks it all as obsolete
RESOURCES += \
    translations.qrc
