#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <QList>
#include <QStyle>
#include <QLayout>

#include "scribblearea.h"


class MainWindow : public QMainWindow
{
        Q_OBJECT

    private:

        // WIDGETS
        QTabWidget *tabs;

        // ACTIONS
        QAction *createImageAction;
        QAction *openFileAction;
        QAction *closeFileAction;

        QList<QAction *> saveAsActions;

        QAction *penColorAction;
        QAction *penWidthAction;
        QAction *clearScreenAction;

        QAction *printAction;
        QAction *quitAppAction;
        QAction *aboutAppAction;
        QAction *aboutQtAction;

        // MENUS
        QMenu *generalMenu;
        QMenu *fileMenu;
        QMenu *saveAsMenu;
        QMenu *optionMenu;
        QMenu *helpMenu;

        // TOOL BARS
        QToolBar *mainToolBar;
        QToolBar *toolBox; // contains drawing tools
        QToolBar *settingToolBar; // settings for drawing tools

        // STATUS BAR
        QStatusBar *windowStatusBar;

        // LAYOUTS
        //QGridLayout *mainWindowLayout;
        QHBoxLayout *mainToolBarLayout;
        QHBoxLayout *statusBarLayout;

        // WINDOW INIT
        void createActions();
        void createMenus();
        void createMainToolBar();
        void createToolBox();
        void createSettingToolBar();
        void createStatusBar();

        // OTHER METHODS
        bool mayBeSave();
        bool saveFile(const QByteArray &fileFormat);

    protected:
        void closeEvent(QCloseEvent *event) override;

    private slots:
        void createNewImage(); // create new blank image
        void open(); // open existing image
        void save();
        void printDocument();
        void penColor();
        void penWidth();
        void about();
        void clearScribbleArea();
        void closeIndexedTab(int);
        void updateTabTitle(QString const& title);

    signals:
        void fileSaved(QString const& fileName);


    public:
        explicit MainWindow(QWidget *parent = nullptr);
        QWidget *createTab();
        QWidget *createTab(QString const& fileName);
        ScribbleArea* currentScribbleArea();


};

#endif // MAINWINDOW_H
