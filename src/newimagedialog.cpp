#include <cmath>
#include "include/newimagedialog.h"
#include "ui_newimagedialog.h"

NewImageDialog::NewImageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewImageDialog)
{
    ui->setupUi(this);
    m_imageWidth = ui->widthSpinBox->value();
    m_imageHeight = ui->heightSpinBox->value();
    m_aspectRatio = (double)m_imageWidth / (double)m_imageHeight;
    m_lockAspectRatio = false;

    connect(ui->widthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setImageWidth(int)));
    connect(ui->heightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setImageHeight(int)));
    connect(ui->aspectRatioCheckBox, SIGNAL(stateChanged(int)), this, SLOT(lockAspectRatio(int)));

    // Compute Aspect Ratio if width or height is modified and if the aspect ratio is not locked
    connect(ui->widthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(computeAspectRatio()));
    connect(ui->heightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(computeAspectRatio()));

    // Compute new width/height
    connect(ui->widthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(computeNewHeight()));
    connect(ui->heightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(computeNewWidth()));

    // Display new values in widget
    connect(ui->widthSpinBox, SIGNAL(editingFinished()), this, SLOT(updateHeightValue()));
    connect(ui->heightSpinBox, SIGNAL(editingFinished()), this, SLOT(updateWidthValue()));
}

NewImageDialog::~NewImageDialog()
{
    delete ui;
}


//------------------------------------------------------------------------------
//  SLOTS
//------------------------------------------------------------------------------
void NewImageDialog::setImageWidth(int width){
    m_imageWidth = width;
}

void NewImageDialog::setImageHeight(int height){
    m_imageHeight = height;
}

void NewImageDialog::lockAspectRatio(int checkBoxState){
    if(checkBoxState == 0){
        m_lockAspectRatio = false;
    }else{
        m_lockAspectRatio = true;
    }
}

void NewImageDialog::computeAspectRatio(){
    if(!m_lockAspectRatio) m_aspectRatio = (double)m_imageWidth / (double)m_imageHeight;
}

// Compute new height based on the aspect ratio if the user modifies width
void NewImageDialog::computeNewHeight(){
    if(m_lockAspectRatio) m_imageHeight = (int)round( (double)m_imageWidth / m_aspectRatio );
}

// Compute new width based on the aspect ratio if the user modifies height
void NewImageDialog::computeNewWidth(){
    if(m_lockAspectRatio) m_imageWidth = (int)( (double)m_imageHeight * m_aspectRatio );
}

void NewImageDialog::updateHeightValue(){
    if(m_lockAspectRatio) ui->heightSpinBox->setValue(m_imageHeight);
}

void NewImageDialog::updateWidthValue(){
    if(m_lockAspectRatio) ui->widthSpinBox->setValue(m_imageWidth);
}

