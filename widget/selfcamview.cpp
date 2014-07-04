#include "selfcamview.h"
#include <QActionGroup>
#include <QMessageBox>
#include <QCloseEvent>
#include <QShowEvent>
#include <QVideoFrame>

#include "videosurface.h"
#include "widget.h"

SelfCamView::SelfCamView(Camera* Cam, QWidget* parent)
    : QWidget(parent), displayLabel{new QLabel},
      mainLayout{new QHBoxLayout()}, cam(Cam)
{
    setLayout(mainLayout);
    setWindowTitle(SelfCamView::tr("Tox video test","Title of the window to test the video/webcam"));
    setMinimumSize(320,240);

    updateDisplayTimer.setInterval(5);
    updateDisplayTimer.setSingleShot(false);

    displayLabel->setScaledContents(true);

    mainLayout->addWidget(displayLabel);

    connect(&updateDisplayTimer, SIGNAL(timeout()), this, SLOT(updateDisplay()));
}

SelfCamView::~SelfCamView()
{
}

void SelfCamView::closeEvent(QCloseEvent* event)
{
    cam->unsuscribe();
    updateDisplayTimer.stop();
    event->accept();
}

void SelfCamView::showEvent(QShowEvent* event)
{
    cam->suscribe();
    updateDisplayTimer.start();
    event->accept();
}

void SelfCamView::updateDisplay()
{
    displayLabel->setPixmap(QPixmap::fromImage(cam->getLastImage()));
}

