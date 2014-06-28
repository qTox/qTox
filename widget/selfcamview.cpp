#include "selfcamview.h"
#include <QActionGroup>
#include <QMessageBox>
#include <QCloseEvent>
#include <QShowEvent>

SelfCamView::SelfCamView(QWidget* parent)
    : QWidget(parent), camera(nullptr), mainLayout{new QHBoxLayout()}
{
    setLayout(mainLayout);
    setWindowTitle("Tox video test");
    setMinimumSize(320,240);

    QByteArray cameraDevice;

    /*
    QActionGroup *videoDevicesGroup = new QActionGroup(this);
    videoDevicesGroup->setExclusive(true);
    foreach(const QByteArray &deviceName, QCamera::availableDevices()) {
        QString description = camera->deviceDescription(deviceName);
        QAction *videoDeviceAction = new QAction(description, videoDevicesGroup);
        videoDeviceAction->setCheckable(true);
        videoDeviceAction->setData(QVariant(deviceName));
        if (cameraDevice.isEmpty()) {
            cameraDevice = deviceName;
            videoDeviceAction->setChecked(true);
        }
        ui->menuDevices->addAction(videoDeviceAction);
    }

    connect(videoDevicesGroup, SIGNAL(triggered(QAction*)), SLOT(updateCameraDevice(QAction*)));
    */

    viewfinder = new QCameraViewfinder(this);
    mainLayout->addWidget(viewfinder);
    viewfinder->show();

    setCamera(cameraDevice);
}

SelfCamView::~SelfCamView()
{
    delete camera;
}

void SelfCamView::setCamera(const QByteArray &cameraDevice)
{
    delete camera;

    if (cameraDevice.isEmpty())
        camera = new QCamera;
    else
        camera = new QCamera(cameraDevice);

    //connect(camera, SIGNAL(stateChanged(QCamera::State)), this, SLOT(updateCameraState(QCamera::State)));
    connect(camera, SIGNAL(error(QCamera::Error)), this, SLOT(displayCameraError()));

    camera->setViewfinder(viewfinder);

    //updateCameraState(camera->state());

    camera->setCaptureMode(QCamera::CaptureVideo);
}

void SelfCamView::displayCameraError()
{
    QMessageBox::warning(this, tr("Camera error"), camera->errorString());
}

void SelfCamView::closeEvent(QCloseEvent* event)
{
    camera->stop();
    event->accept();
}

void SelfCamView::showEvent(QShowEvent* event)
{
    camera->start();
    event->accept();
}
