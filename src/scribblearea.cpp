#include "scribblearea.h"

// Constructor
ScribbleArea::ScribbleArea(QPen *penTool, QD::Mode mode, QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StaticContents);
    setCursor(QCursor(Qt::CursorShape::CrossCursor));
    m_modified = false;
    m_scribbling = false;
    m_saved = false;
    m_isTransparent = false;
    m_imageOpened = false;

    m_scaleFactor = 1.0;
    m_pen = penTool;
    m_filePath = new QString();

    // Set Rubber band style
    m_rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    m_rubberBand->setWindowOpacity(0.0); //TODO : Test it  

    //--- Create a new default image (1px by 1px)---
    QImage newImage(QSize(1,1), QImage::Format_ARGB32);
    newImage.fill(QColor(255,255,255));
    m_image = newImage;

    // Label/Container
    m_pixmapContainer = new QLabel(this);
    m_pixmapContainer->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_pixmapContainer->setScaledContents(true);

    // Pixmap
    QPixmap m_pixmap;
    m_pixmap.convertFromImage(m_image);
    m_pixmapContainer->setPixmap(m_pixmap);

    // Set default mode
    m_selectedMode = mode;
}

//---------------------------------------------------------------------------------
//  PUBLIC METHODS
//----------------------------------------------------------------------------------
void ScribbleArea::createImage(QSize const& size, bool isBackgroundTransparent)
{
    QImage newImage(size, QImage::Format_ARGB32);

    if(!isBackgroundTransparent) {
        newImage.fill(QColor(255,255,255));
    }
    else {
        newImage.fill(QColor(255,255,255,0));
        m_isTransparent = true;
    }
    m_image = newImage;

    updatePixmap();
}

bool ScribbleArea::openImage(const QString &fileName)
{
    QImage loadedImage;

    if(!loadedImage.load(fileName))
    {
        return false;
    }

    m_image = addAlphaChannel(loadedImage);
    m_isTransparent = true;
    m_filePath = new QString(fileName);
    m_imageOpened = true;

    updatePixmap();
    update();
    return true;
}

// TODO: Check if the format supports alpha channel. Otherwise, we must replace black pixels with white pixels.
bool ScribbleArea::saveImage(const QString &fileName, const char *fileFormat)
{
    QImage visibleImage = m_image;

    if (visibleImage.save(fileName, fileFormat))
    {
        m_modified = false;
        m_saved = true;
        m_filePath = new QString(fileName);
        return true;
    }
    else
    {
        return false;
    }
}

void ScribbleArea::clearImage()
{
    if(m_isTransparent) m_image.fill(QColor(255,255,255,0));
    else m_image.fill(QColor(255,255,255));
    m_modified = true;
    update();
}

QString* ScribbleArea::getFilePath()
{
    return m_filePath;
}

void ScribbleArea::setMode(QD::Mode mode)
{
    m_selectedMode = mode;
}

QD::Mode ScribbleArea::mode() const
{
    return m_selectedMode;
}

// TODO : Manage scaling and zoom
void ScribbleArea::scale(double factor)
{
    m_scaleFactor *= factor;

    this->resize(m_scaleFactor * this->size());

    //adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    //adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    //zoomInAct->setEnabled(scaleFactor < 3.0);
    //zoomOutAct->setEnabled(scaleFactor > 0.333);
}

double ScribbleArea::scaleFactor() const
{
    return m_scaleFactor;
}

void ScribbleArea::print()
{
#if QT_CONFIG(printdialog)
    QPrinter printer(QPrinter::HighResolution);

    QPrintDialog printDialog(&printer, this);

    if(printDialog.exec() == QDialog::Accepted)
    {
        QPainter painter(&printer);
        QRect rect = painter.viewport(); // Rectangle defining the area of the painter
        QSize size = m_image.size(); // Image size
        size.scale(rect.size(), Qt::KeepAspectRatio); // Scaling the image for the rectangle (sheet of paper)

        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(m_image.rect());
        painter.drawImage(0, 0, m_image);
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
        switch(m_selectedMode)
        {
            case QD::DRAW:
                m_lastPoint = event->pos(); // Get pos of the mouse cursor
                m_scribbling = true;
                break;
            case QD::SELECT:
                m_lastPoint = event->pos();
                m_scribbling = true;
                if (!m_rubberBand)
                    m_rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
                m_rubberBand->setGeometry( QRect(m_lastPoint, QSize()) );
                m_rubberBand->show();
                break;
            case QD::FILL:
                break;
            case QD::ERASE:
                m_lastPoint = event->pos(); // Get pos of the mouse cursor
                m_scribbling = true;
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
    if((event->buttons() & Qt::LeftButton) && m_scribbling)
    {
        switch(m_selectedMode)
        {
            case QD::DRAW:
                drawLineTo(event->pos());
                break;
            case QD::SELECT:
                m_rubberBand->setGeometry(QRect(m_lastPoint, event->pos()).normalized());
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
    if((event->buttons() == Qt::LeftButton) && m_scribbling)
    {
        switch(m_selectedMode)
        {
            case QD::DRAW:
                drawLineTo(event->pos());
                m_scribbling = false;
                break;
            case QD::SELECT:
                m_rubberBand->hide();
                m_scribbling = false;
                // determine selection, for example using QRect::intersects()
                // and QRect::contains().
                break;
            case QD::FILL:
                break;
            case QD::ERASE:
                eraseTo(event->pos());
                m_scribbling = false;
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
    QRect updatedRect = event->rect(); // Draw the rectangular area where we currently draw on
    //painter.drawPixmap(updatedRect, m_pixmap, updatedRect); // Draw the rectangle where the image needs to be updated

    m_pixmap.convertFromImage(m_image);
    m_pixmapContainer->setPixmap(m_pixmap);
    m_pixmapContainer->update(updatedRect);
}

void ScribbleArea::resizeEvent(QResizeEvent *event)
{
    // Resize scribble area and the widget with the tiled background for transparency
    setMinimumSize( m_pixmap.size() ); // Resize drawing area
    parentWidget()->setMaximumSize( QSize(m_pixmap.width()+2*QD::BORDER_SIZE, m_pixmap.height()+2*QD::BORDER_SIZE) ); // Resize parent Widget

    m_pixmapContainer->resize(m_pixmap.size()); // Resize the image container
    update();
    QWidget::resizeEvent(event);
}

//----------------------------------------------------------------------------------
//  PRIVATE METHODS
//----------------------------------------------------------------------------------
void ScribbleArea::updatePixmap(){
    m_pixmap.convertFromImage(m_image);
}

void ScribbleArea::drawLineTo(const QPoint &endPoint)
{
    QPainter painter(&m_image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(*m_pen);
    painter.drawLine(m_lastPoint, endPoint);
    m_modified = true;

    int rad = (m_pen->width() / 2) + 2;
    update( QRect(m_lastPoint, endPoint).normalized().adjusted(-rad, -rad, +rad, +rad) );
    m_lastPoint = endPoint;
}

void ScribbleArea::eraseTo(const QPoint &endPoint)
{
    QPainter painter(&m_image);
    QPen eraser(*m_pen);

    painter.setPen(eraser);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setCompositionMode(QPainter::CompositionMode_Clear);

    painter.drawLine(m_lastPoint, endPoint);
    m_modified = true;

    update();
    m_lastPoint = endPoint;
}

// Copy an image into a new image with alpha channel
QImage ScribbleArea::addAlphaChannel(QImage const& image)
{
    // Create a new white image
    QImage newImage(image.size(), QImage::Format_ARGB32);
    newImage.fill(QColor(255, 255, 255));

    // Draw the image onto the newImage. It allows to avoid black areas
    QPainter painter(&newImage);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.drawImage(QPoint(0, 0), image);

    return newImage;
}

// DEPRECATED
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
