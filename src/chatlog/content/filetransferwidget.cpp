#include "filetransferwidget.h"
#include "ui_filetransferwidget.h"

#include <QMouseEvent>
#include <QDebug>

FileTransferWidget::FileTransferWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileTransferWidget)
{
    ui->setupUi(this);

    setFixedHeight(100);
}

FileTransferWidget::~FileTransferWidget()
{
    delete ui;
}

void FileTransferWidget::on_pushButton_2_clicked()
{
    qDebug() << "Button Cancel Clicked";
}

void FileTransferWidget::on_pushButton_clicked()
{
    qDebug() << "Button Resume Clicked";
}

void FileTransferWidget::on_pushButton_2_pressed()
{
    qDebug() << "Button Resume Clicked";
}
