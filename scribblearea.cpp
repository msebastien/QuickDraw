#include "scribblearea.h"

#define APP_NAME "QuickDraw"

// Constructor
ScribbleArea::ScribbleArea(QPen *penTool, QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StaticContents);
    modified = false;
    scribbling = false;
    saved = false;
    //myPenWidth = 1;
    //myPenColor = Qt::blue;
    pen = penTool;
    filePath = new QString();
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
    image.fill(qRgb(255,255,255));
    modified = true;
    update();
}

QString* ScribbleArea::getFilePath()
{
    return filePath;
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
        lastPoint = event->pos(); // Get pos of the mouse cursor
        scribbling = true;
    }
}

void ScribbleArea::mouseMoveEvent(QMouseEvent *event)
{
    if((event->buttons() & Qt::LeftButton) && scribbling)
        drawLineTo(event->pos());
}

void ScribbleArea::mouseReleaseEvent(QMouseEvent *event)
{
    if((event->buttons() == Qt::LeftButton) && scribbling)
    {
        drawLineTo(event->pos());
        scribbling = false;
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

void ScribbleArea::resizeImage(QImage *image, const QSize &newSize)
{
    if(image->size() == newSize)
        return;

    // Create a new white image
    QImage newImage(newSize, QImage::Format_RGB32);
    newImage.fill( qRgb(255, 255, 255) );

    // Draw the image onto the newImage. It allows to avoid black areas
    // when resizing like with QImage::copy()

    QPainter painter(&newImage);
    painter.drawImage(QPoint(0,0), *image);

    *image = newImage;
}
