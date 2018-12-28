#include "scribblearea.h"

#define APP_NAME "QuickDraw"


// Constructor
ScribbleArea::ScribbleArea(QPen *penTool, QD::Mode mode, QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StaticContents);
    setCursor(QCursor(Qt::CursorShape::CrossCursor));
    m_modified = false;
    m_scribbling = false;
    m_saved = false;
    m_isTransparent = false;

    m_pen = penTool;
    m_filePath = new QString();

    // Set Rubber band style
    m_rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    m_rubberBand->setWindowOpacity(0.0); //TODO : Test it

    m_imageOpened = false;

    //--- Create a new default image (1px by 1px)---
    QImage newImage(QSize(1,1), QImage::Format_ARGB32);
    newImage.fill(QColor(255,255,255));
    m_image = newImage;

    // Set default mode
    m_selectedMode = mode;
}

//---------------------------------------------------------------------------------
//  PUBLIC METHODS
//----------------------------------------------------------------------------------
void ScribbleArea::createImage(QSize const& size, bool isBackgroundTransparent){
    QImage newImage(size, QImage::Format_ARGB32);

    if(!isBackgroundTransparent) {
        newImage.fill(QColor(255,255,255));
    }
    else {
        newImage.fill(QColor(255,255,255,0)); m_isTransparent = true;
    }

    m_image = newImage;
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

    m_image = loadedImage;
    m_modified = false;
    m_filePath = new QString(fileName);
    m_imageOpened = true;
    update();
    return true;
}

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

QD::Mode ScribbleArea::mode()
{
    return m_selectedMode;

}

void ScribbleArea::resizeScribbleArea(QSize const& size)
{
    setMinimumSize(size); // Resize image/drawing area
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
    QRect dirtyRect = event->rect(); // Draw the rectangular area where we currently draw on
    painter.drawImage(dirtyRect, m_image, dirtyRect); // Draw the rectangle where the image needs to be updated
}

void ScribbleArea::resizeEvent(QResizeEvent *event)
{

    int newWidth = qMax(width()-2*QD::BORDER_SIZE, m_image.width()-2*QD::BORDER_SIZE);
    int newHeight = qMax(height()-2*QD::BORDER_SIZE, m_image.height()-2*QD::BORDER_SIZE);

    if(width() > m_image.width() || height() > m_image.height())
    {
        if(m_imageOpened)
        {
            newWidth = qMin(width()-2*QD::BORDER_SIZE, m_image.width()-2*QD::BORDER_SIZE);
            newHeight = qMin(height()-2*QD::BORDER_SIZE, m_image.height()-2*QD::BORDER_SIZE);
        }
    }
    else if(width() < m_image.width())
    {
        setMinimumWidth(newWidth);
    }
    else if(height() < m_image.height())
    {
        setMinimumHeight(newHeight);
    }

    resizeScribbleArea(QSize(m_image.width(), m_image.height()));
    update();
    QWidget::resizeEvent(event);
}

//----------------------------------------------------------------------------------
//  PRIVATE METHODS
//----------------------------------------------------------------------------------
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

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    painter.setPen(*m_pen);

    painter.drawLine(m_lastPoint, endPoint);
    m_modified = true;

    update();
    m_lastPoint = endPoint;
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


