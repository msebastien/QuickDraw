#ifndef NEWIMAGEDIALOG_H
#define NEWIMAGEDIALOG_H

#include <QDialog>
#include "mainwindow.h"

namespace Ui {
    class NewImageDialog;
}

class NewImageDialog : public QDialog
{
        Q_OBJECT

    private:
        Ui::NewImageDialog *ui;

        MainWindow* m_parent;
        int m_imageWidth;
        int m_imageHeight;
        double m_aspectRatio;
        bool m_aspectRatioLocked;
        bool m_transparentBackground;


    public:
        explicit NewImageDialog(QWidget *parent = nullptr);
        ~NewImageDialog();

    private slots:
        void setImageWidth(int width);
        void setImageHeight(int height);
        void lockAspectRatio(int checkBoxState);

        void computeAspectRatio();
        void computeNewHeight();
        void computeNewWidth();

        void updateHeightValue();
        void updateWidthValue();

        void setImageBackground(QString background);
        void createImage();
};

#endif // NEWIMAGEDIALOG_H
