#include "include/scribblearea.h"

#define APP_NAME "QuickDraw"


// Constructor
ScribbleArea::ScribbleArea(QPen *penTool, QD::Mode mode, QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StaticContents);
    setCursor(QCursor(Qt::CursorShape::CrossCursor));
    modified = false;
    scribbling = false;
    saved = false;

    pen = penTool;
    filePath = new QString();

    // Set Rubber band style
    rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    rubberBand->setWindowOpacity(0.0);

    imageOpened = false;

    //--- Create a new default image (1px by 1px)---
    QImage newImage(QSize(1,1), QImage::Format_ARGB32);
    newImage.fill(QColor(255, 255, 255));
    image = newImage;

    // Set default mode
    selectedMode = mode;
}

//---------------------------------------------------------------------------------
//  PUBLIC METHODS
//----------------------------------------------------------------------------------
void ScribbleArea::createImage(QSize const& size, bool isBackgroundTransparent){
    QImage newImage(size, QImage::Format_ARGB32);

    if(!isBackgroundTransparent) newImage.fill(QColor(255, 255, 255));
    else newImage.fill(QColor(255,255,255,0));

    resizeImage(&newImage, QSize(size.width()-2*QD::BORDER_SIZE, size.height()-2*QD::BORDER_SIZE));
    image = newImage;
}

bool ScribbleArea::openImage(const QString &fileName)
{
    QImage loadedImage;
    if(!loadedImage.load(fileName))
    {
        return false;
    }

    QSize newSize = loadedImage.size().expandedTo( QSize(width()-2*QD::BORDER_SIZE, height()-2*QD::BORDER_SIZE) );
    resizeImage(&loadedImage, newSize);

    image = loadedImage;
    modified = false;
    filePath = new QString(fileName);
    imageOpened = true;
    update();
    return true;
}

bool ScribbleArea::saveImage(const QString &fileName, const char *fileFormat)
{
    QImage visibleImage = image;

    resizeImage(&visibleImage, QSize(width()-2*QD::BORDER_SIZE, height()-2*QD::BORDER_SIZE)); // Resize image as it is the same size as the widget

    if (visibleImage.save(fileName, fileFormat))
    {
        modified = false;
        saved = true;
        filePath = new QString(fileName);
        return true;
    }
    else
    {
        return false;
    }
}


void ScribbleArea::clearImage()
{
    image.fill(QColor(255,255,255));
    modified = true;
    update();
}

QString* ScribbleArea::getFilePath()
{
    return filePath;
}

void ScribbleArea::setMode(QD::Mode mode)
{
    selectedMode = mode;
}

QD::Mode ScribbleArea::mode()
{
    return selectedMode;

}

void ScribbleArea::resizeScribbleArea(QSize const& size)
{
    setMinimumSize(size); // Resize image
    parentWidget()->setMaximumSize( QSize(size.width()+2*QD::BORDER_SIZE, size.height()+2*QD::BORDER_SIZE) ); // Resize Widget
}

//---------------------------------------------------------------------------------
//  SLOTS
//---------------------------------------------------------------------------------
void ScribbleArea::print()
{
#if QT_CONFIG(printdialog)
    QPrinter printer(QPrinter::HighResolution);

    QPrintDialog printDialog(&printer, this);

    if(printDialog.exec() == QDialog::Accepted)
    {
        QPainter painter(&printer);
        QRect rect = painter.viewport(); // Rectangle defining the area of the painter
        QSize size = image.size(); // Image size
        size.scale(rect.size(), Qt::KeepAspectRatio); // Scaling the image for the rectangle (sheet of paper)

        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(image.rect());
        painter.drawImage(0, 0, image);
    }
#endif //QT_CONFIG(printdialog)
}

//----------------------------------------------------------------------------------
//  EVENTS (PROTECTED METHODS)
//----------------------------------------------------------------------------------
void ScribbleArea::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        switch(selectedMode)
        {
            case QD::DRAW:
                lastPoint = event->pos(); // Get pos of the mouse cursor
                scribbling = true;
                break;
            case QD::SELECT:
                lastPoint = event->pos();
                scribbling = true;
                if (!rubberBand)
                    rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
                rubberBand->setGeometry( QRect(lastPoint, QSize()) );
                rubberBand->show();
                break;
            case QD::FILL:
                break;
            case QD::ERASE:
                lastPoint = event->pos(); // Get pos of the mouse cursor
                scribbling = true;
                break;
            case QD::ZOOM:
                break;
            case QD::PIPETTE:
                break;
        }
    }
}

void ScribbleArea::mouseMoveEvent(QMouseEvent *event)
{
    if((event->buttons() & Qt::LeftButton) && scribbling)
    {
        switch(selectedMode)
        {
            case QD::DRAW:
                drawLineTo(event->pos());
                break;
            case QD::SELECT:
                rubberBand->setGeometry(QRect(lastPoint, event->pos()).normalized());
                break;
            case QD::FILL:
                break;
            case QD::ERASE:
                eraseTo(event->pos());
                break;
            case QD::ZOOM:
                break;
            case QD::PIPETTE:
                break;
        }
    }

}

void ScribbleArea::mouseReleaseEvent(QMouseEvent *event)
{
    if((event->buttons() == Qt::LeftButton) && scribbling)
    {
        switch(selectedMode)
        {
            case QD::DRAW:
                drawLineTo(event->pos());
                scribbling = false;
                break;
            case QD::SELECT:
                rubberBand->hide();
                scribbling = false;
                // determine selection, for example using QRect::intersects()
                // and QRect::contains().
                break;
            case QD::FILL:
                break;
            case QD::ERASE:
                eraseTo(event->pos());
                scribbling = false;
                break;
            case QD::ZOOM:
                break;
            case QD::PIPETTE:
                break;
        }
    }
}

void ScribbleArea::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QRect dirtyRect = event->rect(); // Draw the rectangular area where we currently draw on
    painter.drawImage(dirtyRect, image, dirtyRect); // Draw the rectangle where the image needs to be updated
}

void ScribbleArea::resizeEvent(QResizeEvent *event)
{

    int newWidth = qMax(width()-2*QD::BORDER_SIZE, image.width()-2*QD::BORDER_SIZE);
    int newHeight = qMax(height()-2*QD::BORDER_SIZE, image.height()-2*QD::BORDER_SIZE);



    if(width() > image.width() || height() > image.height())
    {
        if(imageOpened)
        {
            newWidth = qMin(width()-2*QD::BORDER_SIZE, image.width()-2*QD::BORDER_SIZE);
            newHeight = qMin(height()-2*QD::BORDER_SIZE, image.height()-2*QD::BORDER_SIZE);
        }
    }
    else if(width() < image.width())
    {
        setMinimumWidth(newWidth);
    }
    else if(height() < image.height())
    {
        setMinimumHeight(newHeight);
    }

    resizeScribbleArea(QSize(image.width(), image.height()));
    update();
    QWidget::resizeEvent(event);
}

//----------------------------------------------------------------------------------
//  PRIVATE METHODS
//----------------------------------------------------------------------------------
void ScribbleArea::drawLineTo(const QPoint &endPoint)
{
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(*pen);
    painter.drawLine(lastPoint, endPoint);
    modified = true;

    int rad = (pen->width() / 2) + 2;
    update( QRect(lastPoint, endPoint).normalized().adjusted(-rad, -rad, +rad, +rad) );
    lastPoint = endPoint;
}

void ScribbleArea::eraseTo(const QPoint &endPoint)
{
    QPainter painter(&image);

    painter.setRenderHint(QPainter::Antialiasing, true);
    // Alpha : 0 (Transparent) to 255 (Opaque)
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    painter.setPen(*pen);


    painter.drawLine(lastPoint, endPoint);
    modified = true;

    update();
    lastPoint = endPoint;
}

void ScribbleArea::resizeImage(QImage *image, const QSize &newSize)
{
    if(image->size() == newSize)
        return;

    // Create a new white image
    QImage newImage(newSize, QImage::Format_ARGB32);
    newImage.fill(QColor(255, 255, 255));

    // Draw the image onto the newImage. It allows to avoid black areas
    // when resizing like with QImage::copy()

    QPainter painter(&newImage);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.drawImage(QPoint(0, 0), *image);

    *image = newImage;
}


