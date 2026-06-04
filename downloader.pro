QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG -= app_bundle

TARGET = downloader
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/chunkcalculator.cpp \
    src/chunkmanager.cpp \
    src/downloadsystem.cpp \
    src/fileinfo.cpp \
    src/fileinfomanager.cpp \
    src/filemerger.cpp \
    src/logger.cpp \
    src/maincontroller.cpp \
    src/progresstracker.cpp \
    src/ui.cpp \
    src/validator.cpp \
    src/downloadworker.cpp

HEADERS += \
    include/mainwindow.h \
    include/downloadworker.h \
    include/chunkcalculator.hpp \
    include/chunkmanager.hpp \
    include/downloadsystem.hpp \
    include/fileinfo.hpp \
    include/fileinfomanager.hpp \
    include/filemerger.hpp \
    include/logger.hpp \
    include/maincontroller.hpp \
    include/progresstracker.hpp \
    include/threadautocalculator.hpp \
    include/validator.hpp

INCLUDEPATH += include/

unix:!macx: LIBS += -pthread -lcurl
macx: LIBS += -pthread -lcurl
win32: LIBS += -lws2_32 -lcurl

FORMS += \
    ui/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
