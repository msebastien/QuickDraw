#ifndef NEWIMAGEDIALOG_H
#define NEWIMAGEDIALOG_H

#include <QObject>
#include <QDialog>
//#include <QSpinBox>

namespace Ui {
    class NewImageDialog;
}

class NewImageDialog : public QDialog
{
        Q_OBJECT

    private:
        Ui::NewImageDialog *ui;
        int m_imageWidth;
        int m_imageHeight;
        double m_aspectRatio;
        bool m_lockAspectRatio;

    public:
        explicit NewImageDialog(QWidget *parent = nullptr);
        ~NewImageDialog();

    public slots:
        void setImageWidth(int width);
        void setImageHeight(int height);
        void lockAspectRatio(int checkBoxState);

        void computeAspectRatio();
        void computeNewHeight();
        void computeNewWidth();

        void updateHeightValue();
        void updateWidthValue();
};

#endif // NEWIMAGEDIALOG_H
