#include <QCoreApplication>
#include "mainwindow.h"
#include "scribblearea.h"
#include "newimagedialog.h"

#define APP_NAME "QuickDraw"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    createActions();
    createMenus();
    createMainToolBar();

    // TOOLS
    mode = QD::DRAW;
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
    setWindowIcon( QIcon("ui/icons/mimes/128/image-x-generic.svg") );
    setWindowTitle(APP_NAME);

    connect(this, SIGNAL(fileSaved(QString const&)), this, SLOT(updateTabTitle(QString const&)));
}

// When creating a new image
QWidget* MainWindow::createTab(int width, int height, bool isBackgroundTransparent)
{
    QWidget *tab = new QWidget;

    ScribbleArea *scribbleArea = new ScribbleArea(pen, mode);
    scribbleArea->createImage(QSize(width, height), isBackgroundTransparent);

    QVBoxLayout *tabLayout = new QVBoxLayout;
    tabLayout->setContentsMargins(0, 0, 0, 0);

    /*--ViewPort Area--*/
    QWidget *area = new QWidget;
    QVBoxLayout *areaLayout = new QVBoxLayout;
    areaLayout->setContentsMargins(QD::BORDER_SIZE, QD::BORDER_SIZE, QD::BORDER_SIZE, QD::BORDER_SIZE);
    areaLayout->addWidget(scribbleArea);
    area->setLayout(areaLayout);
    area->setAttribute(Qt::WA_StaticContents);
    area->setObjectName("Viewport");
    area->setStyleSheet( QString("QWidget#Viewport {border: %1px solid black; background: url(ui/images/Transparency10.png); background-repeat: repeat-xy; background-attachment: fixed;}").arg(QD::BORDER_SIZE) );

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setAlignment(Qt::AlignCenter);
    //scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(area);
    /*--End ViewPort Area--*/

    tabLayout->addWidget(scrollArea);
    tab->setLayout(tabLayout);

    return tab;
}
// When opening an existing image
QWidget* MainWindow::createTab(QString const& fileName)
{
    QWidget *tab = new QWidget;

    ScribbleArea *scribbleArea = new ScribbleArea(pen, mode);
    scribbleArea->openImage(fileName);

    QVBoxLayout *tabLayout = new QVBoxLayout;
    tabLayout->setContentsMargins(0, 0, 0, 0);

    /*--ViewPort Area--*/
    QWidget *area = new QWidget;
    QVBoxLayout *areaLayout = new QVBoxLayout;
    areaLayout->setContentsMargins(QD::BORDER_SIZE, QD::BORDER_SIZE, QD::BORDER_SIZE, QD::BORDER_SIZE);
    areaLayout->addWidget(scribbleArea);
    area->setLayout(areaLayout);
    area->setAttribute(Qt::WA_StaticContents);
    area->setObjectName("Viewport");
    area->setStyleSheet( QString("QWidget#Viewport {border: %1px solid black; background: url(ui/images/Transparency10.png); background-repeat: repeat-xy; background-attachment: fixed;}").arg(QD::BORDER_SIZE) );

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setAlignment(Qt::AlignCenter);
    //scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(area);
    /*--End ViewPort Area--*/

    tabLayout->addWidget(scrollArea);
    tab->setLayout(tabLayout);

    return tab;
}

QTabWidget* MainWindow::tabWidget(){
    return tabs;
}

ScribbleArea* MainWindow::currentScribbleArea()
{
    return tabs->currentWidget()->findChild<QScrollArea *>()->findChild<ScribbleArea *>();
}

ScribbleArea* MainWindow::indexedScribbleArea(int index)
{
    return tabs->widget(index)->findChild<QScrollArea *>()->findChild<ScribbleArea *>();
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
        ret = QMessageBox::warning(this, tr("Warning"),
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
        QMessageBox::warning(this, tr("Warning"), tr("The file format has been <b>modified</b>.\n Your image was previously saved in the ")
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
    NewImageDialog newImageDialog(this);
    newImageDialog.exec();
}

void MainWindow::open()
{
    if(mayBeSave())
    {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::currentPath());
        if(!fileName.isEmpty())
        {
            tabs->addTab(createTab(fileName), "*" + fileName.section('/',-1, -1));
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

        QString selectedFormat = QInputDialog::getItem(this, tr("Format selector"),
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
        penColor = newColor;
        pen->setColor(newColor);
    }
}

void MainWindow::changePenWidth()
{
    bool ok;
    int newWidth = QInputDialog::getInt(this, tr("Pen Width"),
                                        tr("Select pen width :"),
                                        pen->width(),
                                        1, 50, 1, &ok);
    if(ok){
        penWidth = newWidth;
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
            tabs->widget(index)->findChild<QScrollArea *>()->findChild<ScribbleArea *>()->close();
            tabs->widget(index)->findChild<QScrollArea *>()->close();
            tabs->widget(index)->findChild<QWidget*>()->close();
            tabs->removeTab(index);
        }
        else
        {
            qApp->quit();
        }
    }

}

void MainWindow::updateTabTitle(QString const& filePath){
    QString fileName = filePath.section('/', -1, -1);
    tabs->setTabText(tabs->currentIndex(), fileName);
}

void MainWindow::selectMode(){
    currentScribbleArea()->setMode(QD::SELECT);
    mode = QD::SELECT; // memorize selected mode in MainWindow
}

void MainWindow::drawMode(){
    currentScribbleArea()->setMode(QD::DRAW);
    pen->setColor(penColor);
    pen->setWidth(penWidth);
    mode = QD::DRAW; // memorize selected mode in MainWindow
}

void MainWindow::fillMode(){
    currentScribbleArea()->setMode(QD::FILL);
    mode = QD::FILL; // memorize selected mode in MainWindow
}

void MainWindow::eraseMode(){
    currentScribbleArea()->setMode(QD::ERASE);
    mode = QD::ERASE; // memorize selected mode in MainWindow
}

//------------------------------------------------------------------------------
//  INIT WINDOW
//------------------------------------------------------------------------------
void MainWindow::createActions()
{
    createImageAction = new QAction(tr("New image"), this);
    createImageAction->setShortcut(QKeySequence("Ctrl+N"));
    createImageAction->setIcon(QIcon( QIcon::fromTheme("document-new", QIcon("ui/icons/actions/24/document-new.svg")) ));
    connect(createImageAction, SIGNAL(triggered()), this, SLOT(createNewImage()));

    openFileAction = new QAction(tr("&Open..."), this);
    openFileAction->setShortcuts(QKeySequence::Open);
    openFileAction->setIcon(QIcon( QIcon::fromTheme("folder-open", QIcon("ui/icons/places/24/folder-open.svg")) ));
    connect(openFileAction, SIGNAL(triggered()), this, SLOT(open()));

    closeFileAction = new QAction(tr("Close image"), this);
    closeFileAction->setIcon(QIcon( QIcon::fromTheme("window-close", QIcon("ui/icons/actions/24/window-close.svg")) ));

    saveAction = new QAction(tr("Save"), this);
    saveAction->setShortcuts(QKeySequence::Save);
    saveAction->setIcon(QIcon( QIcon::fromTheme("document-save", QIcon("ui/icons/mimes/24/text-x-install.svg")) ));
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
    clearScreenAction->setIcon(QIcon("ui/icons/actions/24/edit-clear.svg"));
    connect(clearScreenAction, SIGNAL(triggered()), this, SLOT(clearScribbleArea()));

    printAction = new QAction(tr("Print"), this);
    printAction->setShortcuts(QKeySequence::Print);
    printAction->setIcon(QIcon( QIcon::fromTheme("document-print", QIcon("ui/icons/devices/24/printer.svg")) ));
    connect(printAction, SIGNAL(triggered()), this, SLOT(printDocument()));

    quitAppAction = new QAction(tr("&Quit"), this);
    quitAppAction->setShortcuts(QKeySequence::Quit);
    connect(quitAppAction, SIGNAL(triggered()), this, SLOT(close()));

    aboutAppAction = new QAction(tr("About"), this);
    aboutAppAction->setIcon(QIcon( QIcon::fromTheme("help-about", QIcon("ui/icons/actions/24/help-about.svg")) ));
    connect(aboutAppAction, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAction = new QAction(tr("About Qt"), this);
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    // MODES
    modeActionGroup = new QActionGroup(this);
    modeActionGroup->setExclusive(true);

    selectModeAction = new QAction(tr("Select Mode"), this);
    selectModeAction->setCheckable(true);
    selectModeAction->setShortcut(tr("Ctrl+W"));
    selectModeAction->setIcon(QIcon( QIcon::fromTheme("tool-pointer", QIcon("ui/icons/categories/24/preferences-desktop-accessibility-pointing.svg")) ));
    connect(selectModeAction, SIGNAL(triggered()), this, SLOT(selectMode()));

    drawModeAction = new QAction(tr("Draw Mode"), this);
    drawModeAction->setCheckable(true);
    drawModeAction->setChecked(true);
    drawModeAction->setShortcut(tr("Ctrl+D"));
    drawModeAction->setIcon(QIcon( QIcon::fromTheme("draw-freehand", QIcon("ui/icons/actions/24/draw-freehand.svg")) ));
    connect(drawModeAction, SIGNAL(triggered()), this, SLOT(drawMode()));

    fillModeAction = new QAction(tr("Fill Mode"), this);
    fillModeAction->setCheckable(true);
    fillModeAction->setShortcut(tr("Ctrl+F"));
    fillModeAction->setIcon(QIcon( QIcon::fromTheme("color-fill", QIcon("ui/icons/actions/24/color-fill.svg")) ));
    connect(fillModeAction, SIGNAL(triggered()), this, SLOT(fillMode()));

    eraseModeAction = new QAction(tr("Erase Mode"), this);
    eraseModeAction->setCheckable(true);
    eraseModeAction->setShortcut(tr("Ctrl+E"));
    eraseModeAction->setIcon(QIcon( QIcon::fromTheme("draw-eraser", QIcon("ui/icons/actions/24/draw-eraser.svg")) ));
    connect(eraseModeAction, SIGNAL(triggered()), this, SLOT(eraseMode()));

    modeActionGroup->addAction(selectModeAction);
    modeActionGroup->addAction(drawModeAction);
    modeActionGroup->addAction(fillModeAction);
    modeActionGroup->addAction(eraseModeAction);

}

void MainWindow::createMenus()
{
    saveAsMenu = new QMenu(tr("&Save As"), this);
    foreach (QAction *action, saveAsActions)
        saveAsMenu->addAction(action);

    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(createImageAction);
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
    toolBox->addAction(selectModeAction);
    toolBox->addAction(drawModeAction);
    toolBox->addAction(fillModeAction);
    toolBox->addAction(eraseModeAction);
    toolBox->addSeparator();
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


