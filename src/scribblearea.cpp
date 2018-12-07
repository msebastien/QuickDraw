#include "include/scribblearea.h"

#define APP_NAME "QuickDraw"

// Constructor
ScribbleArea::ScribbleArea(QPen *penTool, QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StaticContents);
    setCursor(QCursor(Qt::CursorShape::CrossCursor));
    modified = false;
    scribbling = false;
    saved = false;
    //myPenWidth = 1;
    //myPenColor = Qt::blue;
    pen = penTool;
    filePath = new QString();

    // Set default mode
    selectedMode = QD::DRAW;
}

//---------------------------------------------------------------------------------
//  PUBLIC METHODS
//----------------------------------------------------------------------------------
bool ScribbleArea::openImage(const QString &fileName)
{
    QImage loadedImage;
    if(!loadedImage.load(fileName))
    {
        return false;
    }
    QSize newSize = loadedImage.size().expandedTo(size());
    resizeImage(&loadedImage, newSize);

    image = loadedImage;
    modified = false;
    filePath = new QString(fileName);
    update();
    return true;
}

bool ScribbleArea::saveImage(const QString &fileName, const char *fileFormat)
{
    QImage visibleImage = image;

    resizeImage(&visibleImage, size()); // Resize image as it is the same size as the widget

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
            case QD::FILL:
                break;
            case QD::ERASE:
                lastPoint = event->pos(); // Get pos of the mouse cursor
                scribbling = true;
                break;
            case QD::ZOOM:
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
            case QD::FILL:
                break;
            case QD::ERASE:
                eraseTo(event->pos());
                break;
            case QD::ZOOM:
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
            case QD::FILL:
                break;
            case QD::ERASE:
                eraseTo(event->pos());
                scribbling = false;
                break;
            case QD::ZOOM:
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
    int newWidth = qMax(width(), image.width());
    int newHeight = qMax(height(), image.height());

    if(width() > image.width() || height() > image.height())
    {
        resizeImage(&image, QSize(newWidth, newHeight));
        setMinimumSize(QSize(newWidth, newHeight));

        update();
    }
    else if(width() < image.width()) // Set the scribble area's minimum size to be equal to the image size
    {
        setMinimumWidth(image.width());
        update();
    }
    else if(height() < image.height())
    {
        setMinimumHeight(image.height());
        update();
    }
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

    //int rad = (myPenWidth / 2) + 2;
    //update( QRect(lastPoint, endPoint).normalized().adjusted(-rad, -rad, +rad, +rad) );
    update();
    lastPoint = endPoint;
}

void ScribbleArea::eraseTo(const QPoint &endPoint)
{
    QPainter painter(&image);

    painter.setRenderHint(QPainter::Antialiasing, true);
    // Alpha : 0 (Transparent) to 255 (Opaque)
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    pen->setColor(QColor(255, 255, 255));
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
    painter.drawImage(QPoint(0,0), *image);

    *image = newImage;
}


