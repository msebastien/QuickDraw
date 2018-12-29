#include <cmath>
#include "newimagedialog.h"
#include "ui_newimagedialog.h"

NewImageDialog::NewImageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewImageDialog)
{
    ui->setupUi(this);
    m_parent = qobject_cast<MainWindow*>(parent); // Get pointer to the Main Window

    m_imageWidth = ui->widthSpinBox->value();
    m_imageHeight = ui->heightSpinBox->value();
    m_aspectRatio = (double)m_imageWidth / (double)m_imageHeight;
    m_aspectRatioLocked = ui->aspectRatioCheckBox->isChecked();
    m_transparentBackground = !(ui->backgroundComboBox->currentText() == tr("White"));

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

    //Background
    connect(ui->backgroundComboBox, SIGNAL(activated(QString)), this, SLOT(setImageBackground(QString)));

    // Create image
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(createImage()));
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
        m_aspectRatioLocked = false;
    }else{
        m_aspectRatioLocked = true;
    }
}

void NewImageDialog::computeAspectRatio(){
    if(!m_aspectRatioLocked) m_aspectRatio = (double)m_imageWidth / (double)m_imageHeight;
}

// Compute new height based on the aspect ratio if the user modifies width
void NewImageDialog::computeNewHeight(){
    if(m_aspectRatioLocked) m_imageHeight = (int)round( (double)m_imageWidth / m_aspectRatio );
}

// Compute new width based on the aspect ratio if the user modifies height
void NewImageDialog::computeNewWidth(){
    if(m_aspectRatioLocked) m_imageWidth = (int)round( (double)m_imageHeight * m_aspectRatio );
}

void NewImageDialog::updateHeightValue(){
    if(m_aspectRatioLocked) ui->heightSpinBox->setValue(m_imageHeight);
}

void NewImageDialog::updateWidthValue(){
    if(m_aspectRatioLocked) ui->widthSpinBox->setValue(m_imageWidth);
}

void NewImageDialog::setImageBackground(QString background){
    if(background == tr("White")) m_transparentBackground = false;
    else m_transparentBackground = true;
}

void NewImageDialog::createImage(){
    m_parent->tabWidget()->addTab(m_parent->createTab(m_imageWidth, m_imageHeight, m_transparentBackground), tr("Untitled"));
}

