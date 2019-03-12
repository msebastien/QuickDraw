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
        QImage addAlphaChannel(QImage const& image);

        bool m_modified; // indicate if the image has been modified
        bool m_scribbling; // indicate if the user is currently drawing
        bool m_saved;
        bool m_imageOpened;
        bool m_isTransparent;

        QString *m_filePath;
        QPen *m_pen;

        QImage m_image;

        QPoint m_lastPoint; // monitor constant changes of the mouse cursor
        QD::Mode m_selectedMode;
        QRubberBand *m_rubberBand;

    public:
        explicit ScribbleArea(QPen *m_pen, QD::Mode mode = QD::DRAW, QWidget *parent = nullptr);
        void createImage(QSize const& size, bool isBackgroundTransparent);
        bool openImage(const QString &fileName);
        bool saveImage(const QString &fileName, const char *fileFormat);
        void clearImage();
        void print();
        bool isModified() const {return m_modified;}
        bool isSaved() const {return m_saved;}
        QString* getFilePath();

        void setMode(QD::Mode mode);
        QD::Mode mode() const;

    protected:
        void mousePressEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void paintEvent(QPaintEvent *event) override; // Update the scribble area (some parts)
        void resizeEvent(QResizeEvent *event) override;

};

#endif // SCRIBBLEAREA_H
