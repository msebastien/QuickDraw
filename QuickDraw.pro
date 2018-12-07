TEMPLATE = app
QT += core widgets svg printsupport

#TARGET =
DEPENDPATH += .
INCLUDEPATH += .

# Input
HEADERS += \
    include/mainwindow.h \
    include/scribblearea.h

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/scribblearea.cpp
