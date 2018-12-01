#ifndef SCRIBBLEAREA_H
#define SCRIBBLEAREA_H

#include <QtWidgets>
#include <QImage>
#include <QPoint>
#include <QColor>

#include <QPrinter>
#include <QPrintDialog>

class ScribbleArea : public QWidget
{
        Q_OBJECT

    private:
        void drawLineTo(const QPoint &endPoint);
        void resizeImage(QImage *image, const QSize &newSize);

        bool modified; // indicated if the image has been modified
        bool scribbling; // indicates if the user is currently drawing
        bool saved;
        QString *filePath;
        //int myPenWidth;
        //QColor myPenColor;
        QPen *pen;
        QImage image;
        QPoint lastPoint; // monitor constant changes of our mouse



    public:
        explicit ScribbleArea(QPen *pen, QWidget *parent = nullptr);
        bool openImage(const QString &fileName);
        bool saveImage(const QString &fileName, const char *fileFormat);
        bool isModified() const {return modified;}
        bool isSaved() const {return saved;}
        QString* getFilePath();

        // Pen
        //void setPenColor(const QColor &newColor);
        //void setPenWidth(int newWidth);
        //QColor penColor() const {return myPenColor;}
        //int penWidth() const {return myPenWidth;}

    public slots:
        void clearImage();
        void print();


    protected:
        void mousePressEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void paintEvent(QPaintEvent *event) override; // Update the scribble area (some parts)
        void resizeEvent(QResizeEvent *event) override;
};

#endif // SCRIBBLEAREA_H
