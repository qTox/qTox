#ifndef SELFCAMVIEW_H
#define SELFCAMVIEW_H

#include <QCamera>
#include <QCameraImageCapture>
#include <QMediaRecorder>
#include <QWidget>
#include <QVideoWidget>
#include <QCameraViewfinder>
#include <QHBoxLayout>

class QCloseEvent;
class QShowEvent;

class SelfCamView : public QWidget
{
    Q_OBJECT

public:
    SelfCamView(QWidget *parent=0);
    ~SelfCamView();

private:
    void closeEvent(QCloseEvent*);
    void showEvent(QShowEvent*);

private slots:
    void setCamera(const QByteArray &cameraDevice);
    void displayCameraError();

private:
    QCamera *camera;
    QCameraViewfinder* viewfinder;
    QHBoxLayout* mainLayout;
};

#endif // SELFCAMVIEW_H
