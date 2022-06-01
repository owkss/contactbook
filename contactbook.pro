TARGET = contactbook
TEMPLATE = app

QT += core gui widgets sql
CONFIG += c++11

win32: COMPILE_DATE = $$system(date /T)
unix:  COMPILE_DATE = $$system(date +%d.%m.%Y)
VERSION = $$sprintf("%1.%2.%3.%4", $$section(COMPILE_DATE, ., 2, 2), $$section(COMPILE_DATE, ., 1, 1), $$section(COMPILE_DATE, ., 0, 0), $$COMMIT_NUMBER)

CONFIG(release, debug|release){
    DESTDIR = release
    MOC_DIR = release/moc
    OBJECTS_DIR = release/obj
    DEFINES += QT_NO_DEBUG_OUTPUT
} else {
    DESTDIR = debug
    MOC_DIR = debug/moc
    OBJECTS_DIR = debug/obj
}

SOURCES += \
    contact.cpp \
    database.cpp \
    functions.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    contact.h \
    database.h \
    functions.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    files.qrc
