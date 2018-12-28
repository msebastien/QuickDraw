#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>

#include "include/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Translation support
    QTranslator translator;
    QString locale = QLocale::system().name().section('_', 0, 0);
    translator.load(QString("quickdraw_") + locale, "translations");
    app.installTranslator(&translator);

    MainWindow mainWindow;

    mainWindow.show();

    return app.exec();
}
