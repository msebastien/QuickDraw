#include "pen.h"

Pen::Pen(QObject *parent) : QObject(parent)
{
    width = 1;
    color = Qt::blue;
}
