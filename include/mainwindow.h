#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <QList>
#include <QStyle>
#include <QLayout>
#include <QDialog>

#include "scribblearea.h"


class MainWindow : public QMainWindow
{
        Q_OBJECT

    private:

        // WIDGETS
        QTabWidget *tabs;

        // TOOLS
        QD::Mode mode;
        int penWidth;
        QColor penColor;
        QPen *pen;

        // ACTIONS
        QAction *createImageAction;
        QAction *openFileAction;
        QAction *closeFileAction;

        QAction *saveAction;
        QList<QAction *> saveAsActions;

        QAction *penAction;
        QAction *penColorAction;
        QAction *penWidthAction;
        QAction *clearScreenAction;

        QActionGroup *modeActionGroup;
        QAction *selectModeAction;
        QAction *drawModeAction;
        QAction *fillModeAction;
        QAction *eraseModeAction;
        QAction *zoomModeAction;

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
        QHBoxLayout *mainToolBarLayout;
        QHBoxLayout *statusBarLayout;

        // WINDOW INIT
        void createActions();
        void createMenus();
        void createMainToolBar();
        void createTools();
        void createToolBox();
        void createSettingToolBar();
        void createStatusBar();

        // OTHER METHODS
        bool mayBeSave();
        bool saveFile(); // Save a file which exists
        bool saveAsFile(const QByteArray &fileFormat);

    protected:
        void closeEvent(QCloseEvent *event) override;

    public slots:
        void createNewImage(); // create new blank image
        void open(); // open existing image
        void save();
        void saveAs();
        void printDocument();
        void changePenColor();
        void changePenWidth();
        void about();
        void clearScribbleArea();
        void closeIndexedTab(int);
        void updateTabTitle(QString const& title);

        void selectMode();
        void drawMode();
        void fillMode();
        void eraseMode();

    signals:
        void fileSaved(QString const& fileName);


    public:
        explicit MainWindow(QWidget *parent = nullptr);
        QWidget *createTab(int width = 600, int height = 450, bool isBackgroundTransparent = false); // Create a new tab with a new blank image
        QWidget *createTab(QString const& fileName); // Create a new tab with an existing image
        ScribbleArea* currentScribbleArea();
        ScribbleArea* indexedScribbleArea(int index);
        QTabWidget* tabWidget();


};

#endif // MAINWINDOW_H
