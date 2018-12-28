TEMPLATE = app
QT += core widgets svg printsupport

#TARGET =
DEPENDPATH += .
INCLUDEPATH += .

# Input
HEADERS += \
    include/mainwindow.h \
    include/scribblearea.h \
    include/newimagedialog.h

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/scribblearea.cpp \
    src/newimagedialog.cpp

FORMS += \
    ui/forms/newimagedialog.ui

TRANSLATIONS += \
    translations/quickdraw_fr.ts

