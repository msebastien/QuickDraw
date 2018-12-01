#ifndef PEN_H
#define PEN_H

#include <QObject>
#include <QColor>

class Pen : public QObject
{
        Q_OBJECT
    private:
        int width;
        QColor color;

    public:
        explicit Pen(QObject *parent = nullptr);

        void setPenColor(const QColor &newColor);
        void setPenWidth(int newWidth);
        QColor penColor() const {return color;}
        int penWidth() const {return width;}

    signals:

    public slots:
};

#endif // PEN_H
