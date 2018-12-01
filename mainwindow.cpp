#include <QCoreApplication>
#include "mainwindow.h"
#include "scribblearea.h"

#define APP_NAME "QuickDraw"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    createActions();
    createMenus();
    createMainToolBar();

    // TOOLS
    penWidth = 1;
    penColor = Qt::blue;
    createTools();
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
    ScribbleArea *scribbleArea = new ScribbleArea(pen);

    QVBoxLayout *tabLayout = new QVBoxLayout;
    tabLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(scribbleArea);


    tabLayout->addWidget(scrollArea);
    tab->setLayout(tabLayout);

    return tab;
}

QWidget* MainWindow::createTab(QString const& fileName)
{
    QWidget *tab = new QWidget;
    ScribbleArea *scribbleArea = new ScribbleArea(pen);
    scribbleArea->openImage(fileName);

    QVBoxLayout *tabLayout = new QVBoxLayout;
    tabLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(scribbleArea);


    tabLayout->addWidget(scrollArea);
    tab->setLayout(tabLayout);

    return tab;
}

ScribbleArea* MainWindow::currentScribbleArea()
{
    return tabs->currentWidget()->findChild<QScrollArea *>()->findChild<ScribbleArea *>();
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
            return saveAsFile("png");
        }
        else if (ret==QMessageBox::Cancel)
        {
            return false;
        }
    }
    return true; // If the user chooses to discard
}

bool MainWindow::saveFile()
{
    QString fileName, str;
    QByteArray fileFormat;
    bool ret = false;

    fileName = *( currentScribbleArea()->getFilePath() );


    if( !fileName.isEmpty() )
    {
        str = fileName.section('.', 1, 1);
        fileFormat = str.toLatin1();
        emit fileSaved(fileName);
        ret = currentScribbleArea()->saveImage(fileName, fileFormat.constData());
    }
    return ret;
}

bool MainWindow::saveAsFile(QByteArray const& fileFormat)
{
    QString fileName;
    bool ret = false;

    if(currentScribbleArea()->isSaved())
    {
        fileName = *( currentScribbleArea()->getFilePath() );
    }
    else // We choose the location and the file name to save the image
    {
        QString initialPath = QDir::currentPath() + tr("/untitled.") + fileFormat;
        fileName = QFileDialog::getSaveFileName(this, tr("Save As"), initialPath,
                                                        tr("%1 Files (*.%2);; All Files(*)")
                                                        .arg(QString::fromLatin1(fileFormat.toUpper()))
                                                        .arg(QString::fromLatin1(fileFormat)) );
        if(fileName.isEmpty()) return ret;
    }

    if( !fileName.isEmpty() && (fileFormat == fileName.section('.', 1, 1)) )
    {
        emit fileSaved(fileName);
        ret = currentScribbleArea()->saveImage(fileName, fileFormat.constData());
    }else{ // If the format is different than previously when saved
        QMessageBox::warning(this, tr("Warning") + " - " + APP_NAME, tr("The file format has been <b>modified</b>.\n Your image was previously saved in the ")
                             + "<b>" + fileName.section('.', 1, 1).toUpper() + "</b>" + tr(" format.\n A new file will be created."));
        fileName.remove(fileName.section('.',1,1));
        fileName += fileFormat;
        // Create a new file with the new format
        fileName = QFileDialog::getSaveFileName(this, tr("Save As"), fileName,
                                                        tr("%1 Files (*.%2);; All Files(*)")
                                                        .arg(QString::fromLatin1(fileFormat.toUpper()))
                                                        .arg(QString::fromLatin1(fileFormat))
                                                        );
        ret = currentScribbleArea()->saveImage(fileName, fileFormat.constData());
    }
    return ret;
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
            tabs->addTab(createTab(fileName), "*" + fileName);
        }
    }
}

void MainWindow::save()
{
    bool ok = false;
    if(!currentScribbleArea()->getFilePath()->isEmpty())
    {
        saveFile();
    }
    else
    {
        QStringList *formatList = new QStringList();
        foreach (QByteArray format, QImageWriter::supportedImageFormats()){
             formatList->append(format);
        }

        QString selectedFormat = QInputDialog::getItem(this, tr("Format selector") + " - " + APP_NAME,
                                                       "This file has not been opened or previously saved.\n"
                                                       "Choose the file format to save the image.",
                                                       *formatList, 0, false, &ok);
        if (ok)
            saveAsFile(selectedFormat.toLatin1());
    }

}

void MainWindow::saveAs()
{
    QAction *action = qobject_cast<QAction *>(sender());
    QByteArray fileFormat = action->data().toByteArray();
    saveAsFile(fileFormat);
}

void MainWindow::printDocument()
{
    currentScribbleArea()->print();
}

void MainWindow::changePenColor()
{
    QColor newColor = QColorDialog::getColor(pen->color());
    if(newColor.isValid())
    {
        pen->setColor(newColor);
    }
}

void MainWindow::changePenWidth()
{
    bool ok;
    int newWidth = QInputDialog::getInt(this, APP_NAME + tr(" - Pen Width"),
                                        tr("Select pen width :"),
                                        pen->width(),
                                        1, 50, 1, &ok);
    if(ok){
        pen->setWidth(newWidth);
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

    saveAction = new QAction(tr("Save"), this);
    saveAction->setShortcuts(QKeySequence::Save);
    saveAction->setIcon(QIcon( QIcon::fromTheme("document-save", QIcon("icons/mimes/24/text-x-install.svg")) ));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));

    // SAVE AS
    foreach (QByteArray format, QImageWriter::supportedImageFormats())
    {
        QString text = tr("%1...").arg(QString(format).toUpper());

        QAction *action = new QAction(text, this);
        action->setData(format);
        connect(action, SIGNAL(triggered()), this, SLOT(saveAs()));
        saveAsActions.append(action);
    }
    // PEN

    penColorAction = new QAction(tr("&Pen Color..."), this);
    connect(penColorAction, SIGNAL(triggered()), this, SLOT(changePenColor()));

    penWidthAction = new QAction(tr("Pen &Width..."), this);
    connect(penWidthAction, SIGNAL(triggered()), this, SLOT(changePenWidth()));

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

void MainWindow::createTools()
{
    pen = new QPen(penColor, penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
}

void MainWindow::createMainToolBar()
{
    mainToolBar = addToolBar("Main Toolbar");

    mainToolBar->addAction(createImageAction);
    mainToolBar->addAction(openFileAction);
    mainToolBar->addAction(saveAction);
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


