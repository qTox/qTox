#ifndef FILETRANSFERWIDGET_H
#define FILETRANSFERWIDGET_H

#include <QWidget>
#include "../chatlinecontent.h"

namespace Ui {
class FileTransferWidget;
}

class FileTransferWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileTransferWidget(QWidget *parent = 0);
    virtual ~FileTransferWidget();

private:
    Ui::FileTransferWidget *ui;

private slots:
    void on_pushButton_2_clicked();
    void on_pushButton_clicked();
    void on_pushButton_2_pressed();

};

#endif // FILETRANSFERWIDGET_H
