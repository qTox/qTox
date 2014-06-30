#ifndef NETCAMVIEW_H
#define NETCAMVIEW_H

#include <QWidget>
#include <QHBoxLayout>
#include <QTimer>
#include <QLabel>
#include <vpx/vpx_image.h>

class QCloseEvent;
class QShowEvent;
class QPainter;

class NetCamView : public QWidget
{
    Q_OBJECT

public:
    NetCamView(QWidget *parent=0);

public slots:
    void updateDisplay(vpx_image frame);

private:
    QLabel *displayLabel;
    QImage lastFrame;
    QHBoxLayout* mainLayout;
};

#endif // NETCAMVIEW_H
