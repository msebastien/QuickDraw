#ifndef SCRIBBLEAREA_H
#define SCRIBBLEAREA_H

#include <QtWidgets>
#include <QImage>
#include <QPoint>
#include <QColor>

#include <QPrinter>
#include <QPrintDialog>


namespace QD {
    enum Mode{
      DRAW, SELECT, FILL, ERASE, ZOOM, PIPETTE
    };

    const int BORDER_SIZE = 1; // Border size in pixels
}

class ScribbleArea : public QWidget
{
        Q_OBJECT

    private:
        void drawLineTo(const QPoint &endPoint);
        void eraseTo(const QPoint &endPoint);
        void resizeImage(QImage *image, const QSize &newSize);

        bool modified; // indicated if the image has been modified
        bool scribbling; // indicates if the user is currently drawing
        bool saved;
        bool imageOpened;

        double scaleFactor;
        QString *filePath;
        QPen *pen;
        QImage image;
        QPoint lastPoint; // monitor constant changes of our mouse cursor
        QD::Mode selectedMode;
        QRubberBand *rubberBand;
        QWidget * background;

    public:
        explicit ScribbleArea(QPen *pen, QD::Mode mode = QD::DRAW, QWidget *parent = nullptr);
        void resizeScribbleArea(QSize const& size);
        bool openImage(const QString &fileName);
        bool saveImage(const QString &fileName, const char *fileFormat);
        void clearImage();
        void print();
        bool isModified() const {return modified;}
        bool isSaved() const {return saved;}
        QString* getFilePath();

        void setMode(QD::Mode mode);
        QD::Mode mode();

        // Pen
        //void setPenColor(const QColor &newColor);
        //void setPenWidth(int newWidth);
        //QColor penColor() const {return myPenColor;}
        //int penWidth() const {return myPenWidth;}
    protected:
        void mousePressEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void paintEvent(QPaintEvent *event) override; // Update the scribble area (some parts)
        void resizeEvent(QResizeEvent *event) override;

};

#endif // SCRIBBLEAREA_H
