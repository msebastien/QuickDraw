#include <QCoreApplication>
#include "mainwindow.h"
#include "scribblearea.h"

#define APP_NAME "QuickDraw"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    createActions();
    createMenus();
    createMainToolBar();
    createToolBox();
    createSettingToolBar();
    createStatusBar();

    tabs = new QTabWidget;
    tabs->setTabsClosable(true);
    tabs->setMovable(true);
    setCentralWidget(tabs);

    // Create a default tab when opening the window
    tabs->addTab(createTab(), tr("Untitled"));
    connect(tabs, SIGNAL(tabCloseRequested(int)), this, SLOT(closeIndexedTab(int)));


    // Definition of some window's properties
    setMinimumSize(800, 600);
    setWindowIcon( QIcon("icons/mimes/128/image-x-generic.svg") );
    setWindowTitle(APP_NAME);

    connect(this, SIGNAL(fileSaved(QString const&)), this, SLOT(updateTabTitle(QString const&)));
}

QWidget* MainWindow::createTab()
{
    QWidget *tab = new QWidget;
    ScribbleArea *scribbleArea = new ScribbleArea;

    QVBoxLayout *tabLayout = new QVBoxLayout;
    tabLayout->setContentsMargins(0, 0, 0, 0);
    tabLayout->addWidget(scribbleArea);
    tab->setLayout(tabLayout);

    return tab;
}

QWidget* MainWindow::createTab(QString const& fileName)
{
    QWidget *tab = new QWidget;
    ScribbleArea *scribbleArea = new ScribbleArea;
    scribbleArea->openImage(fileName);

    QVBoxLayout *tabLayout = new QVBoxLayout;
    tabLayout->setContentsMargins(0, 0, 0, 0);
    tabLayout->addWidget(scribbleArea);
    tab->setLayout(tabLayout);

    return tab;
}

ScribbleArea* MainWindow::currentScribbleArea()
{
    return tabs->currentWidget()->findChild<ScribbleArea *>();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(mayBeSave())
    {
        event->accept();
    }else{
        event->ignore();
    }
}

// If the user tries to quit the app without saving
bool MainWindow::mayBeSave()
{
    if(currentScribbleArea()->isModified())
    {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, APP_NAME,
                                   tr("The image has been modified.\n"
                                      "Do you want to save your changes ?"),
                                   QMessageBox::Save | QMessageBox::Discard |
                                   QMessageBox::Cancel);
        if(ret == QMessageBox::Save)
        {
            return saveFile("png");
        }
        else if (ret==QMessageBox::Cancel)
        {
            return false;
        }
    }
    return true; // If the user chooses to discard
}

bool MainWindow::saveFile(QByteArray const& fileFormat)
{
    QString initialPath = QDir::currentPath() + tr("/untitled.") + fileFormat;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), initialPath,
                                                    tr("%1 Files (*.%2);; All Files(*)")
                                                    .arg(QString::fromLatin1(fileFormat.toUpper()))
                                                    .arg(QString::fromLatin1(fileFormat))
                                                    );
    if(fileName.isEmpty())
    {
        return false;
    }
    else{
        emit fileSaved(fileName);
        return currentScribbleArea()->saveImage(fileName, fileFormat.constData());
    }
}

//------------------------------------------------------------------------------
//  SLOTS
//------------------------------------------------------------------------------
void MainWindow::createNewImage()
{
    tabs->addTab(createTab(), tr("Untitled"));
}

void MainWindow::open()
{
    if(mayBeSave())
    {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::currentPath());
        if(!fileName.isEmpty())
        {
            tabs->addTab(createTab(fileName), fileName);
        }
    }
}

void MainWindow::save()
{
    QAction *action = qobject_cast<QAction *>(sender());
    QByteArray fileFormat = action->data().toByteArray();
    saveFile(fileFormat);
}

void MainWindow::printDocument()
{
    currentScribbleArea()->print();
}

void MainWindow::penColor()
{
    QColor newColor = QColorDialog::getColor(currentScribbleArea()->penColor());
    if(newColor.isValid())
    {
        currentScribbleArea()->setPenColor(newColor);
    }
}

void MainWindow::penWidth()
{
    bool ok;
    int newWidth = QInputDialog::getInt(this, APP_NAME + tr(" - Pen Width"),
                                        tr("Select pen width :"),
                                        currentScribbleArea()->penWidth(),
                                        1, 50, 1, &ok);
    if(ok){
        currentScribbleArea()->setPenWidth(newWidth);
    }
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About ") + APP_NAME,
                       tr("<p><b>Quickdraw</b> is a simple painting program made by "
                          "<b>Sébastien Maes</b></p>"));
}

void MainWindow::clearScribbleArea()
{
    currentScribbleArea()->clearImage();
}

void MainWindow::closeIndexedTab(int index)
{
    if(mayBeSave()){
        if(tabs->count() > 1)
        {
            tabs->widget(index)->findChild<ScribbleArea *>()->close();
                    tabs->widget(index)->findChild<QWidget*>()->close();
            tabs->removeTab(index);
        }
        else
        {
            qApp->quit();
        }
    }

}

void MainWindow::updateTabTitle(QString const& title){
    tabs->setTabText(tabs->currentIndex(), title);
}

//------------------------------------------------------------------------------
//  INIT WINDOW
//------------------------------------------------------------------------------
void MainWindow::createActions()
{
    createImageAction = new QAction(tr("New image"), this);
    createImageAction->setShortcut(QKeySequence("Ctrl+N"));
    createImageAction->setIcon(QIcon( QIcon::fromTheme("document-new", QIcon("icons/actions/24/document-new.svg")) ));
    connect(createImageAction, SIGNAL(triggered()), this, SLOT(createNewImage()));

    openFileAction = new QAction(tr("&Open..."), this);
    openFileAction->setShortcuts(QKeySequence::Open);
    openFileAction->setIcon(QIcon( QIcon::fromTheme("folder-open", QIcon("icons/places/24/folder-open.svg")) ));
    connect(openFileAction, SIGNAL(triggered()), this, SLOT(open()));

    closeFileAction = new QAction(tr("Close image"), this);
    closeFileAction->setIcon(QIcon( QIcon::fromTheme("window-close", QIcon("icons/actions/24/window-close.svg")) ));

    // SAVE AS
    foreach (QByteArray format, QImageWriter::supportedImageFormats())
    {
        QString text = tr("%1...").arg(QString(format).toUpper());

        QAction *action = new QAction(text, this);
        action->setData(format);
        connect(action, SIGNAL(triggered()), this, SLOT(save()));
        saveAsActions.append(action);
    }

    penColorAction = new QAction(tr("&Pen Color..."), this);
    connect(penColorAction, SIGNAL(triggered()), this, SLOT(penColor()));

    penWidthAction = new QAction(tr("Pen &Width..."), this);
    connect(penWidthAction, SIGNAL(triggered()), this, SLOT(penWidth()));

    clearScreenAction = new QAction(tr("Clear Screen"), this);
    clearScreenAction->setShortcut(tr("Ctrl+L"));
    clearScreenAction->setIcon(QIcon("icons/actions/24/edit-clear.svg"));
    connect(clearScreenAction, SIGNAL(triggered()), this, SLOT(clearScribbleArea()));

    printAction = new QAction(tr("Print"), this);
    printAction->setShortcut(QKeySequence("Ctrl+N"));
    printAction->setIcon(QIcon( QIcon::fromTheme("document-print", QIcon("icons/devices/24/printer.svg")) ));
    connect(printAction, SIGNAL(triggered()), this, SLOT(printDocument()));

    quitAppAction = new QAction(tr("&Quit"), this);
    quitAppAction->setShortcuts(QKeySequence::Quit);
    connect(quitAppAction, SIGNAL(triggered()), this, SLOT(close()));

    aboutAppAction = new QAction(tr("About"), this);
    aboutAppAction->setIcon(QIcon( QIcon::fromTheme("help-about", QIcon("icons/actions/24/help-about.svg")) ));
    connect(aboutAppAction, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAction = new QAction(tr("About Qt"), this);
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

}

void MainWindow::createMenus()
{
    saveAsMenu = new QMenu(tr("&Save As"), this);
    foreach (QAction *action, saveAsActions)
        saveAsMenu->addAction(action);

    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(openFileAction);
    fileMenu->addMenu(saveAsMenu);
    fileMenu->addAction(printAction);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAppAction);

    optionMenu = new QMenu(tr("&Options"), this);
    optionMenu->addAction(penColorAction);
    optionMenu->addAction(penWidthAction);
    optionMenu->addSeparator();
    optionMenu->addAction(clearScreenAction);

    helpMenu = new QMenu(tr("&Help"), this);
    helpMenu->addAction(aboutAppAction);
    helpMenu->addAction(aboutQtAction);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(optionMenu);
    menuBar()->addMenu(helpMenu);
}

void MainWindow::createMainToolBar()
{
    mainToolBar = addToolBar("Main Toolbar");

    mainToolBar->addAction(createImageAction);
    mainToolBar->addAction(openFileAction);
    mainToolBar->addSeparator();
    mainToolBar->addAction(printAction);

}

void MainWindow::createToolBox()
{
    toolBox = new QToolBar("ToolBox");
    addToolBar(Qt::ToolBarArea::LeftToolBarArea, toolBox);
    toolBox->addAction(clearScreenAction);
}

void MainWindow::createSettingToolBar()
{
    settingToolBar = new QToolBar("Setting ToolBar");
    addToolBar(Qt::ToolBarArea::TopToolBarArea, settingToolBar);
    settingToolBar->addAction(penColorAction);
    settingToolBar->addAction(penWidthAction);
}

void MainWindow::createStatusBar()
{
    windowStatusBar = statusBar();
}


