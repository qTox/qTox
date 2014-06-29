#ifndef SELFCAMVIEW_H
#define SELFCAMVIEW_H

#include <QWidget>
#include <QHBoxLayout>
#include <QTimer>
#include <QLabel>
#include "camera.h"

class QCloseEvent;
class QShowEvent;
class QPainter;

class SelfCamView : public QWidget
{
    Q_OBJECT

public:
    SelfCamView(Camera* Cam, QWidget *parent=0);
    ~SelfCamView();

private slots:
    void updateDisplay();

private:
    void closeEvent(QCloseEvent*);
    void showEvent(QShowEvent*);
    void paint(QPainter *painter);

private:
    QLabel *displayLabel;
    QHBoxLayout* mainLayout;
    Camera* cam;
    QTimer updateDisplayTimer;
};

#endif // SELFCAMVIEW_H
